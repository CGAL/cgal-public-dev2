// Author: Keyu CHEN.
// In this test we compute maximum entropy coordinates for ~2400 strictly interior points
// with respect to a triangle and compare them with those from triangle coordinates.
// They should be the same. But currently we are using sqrt() and exp() functions in Maximum_entropy_2 class.
// So there is very small inconsistency in our results (less than 1e-5).

// Todo: Fix Maximum_entropy_2 class with exact kernel.

#include <cassert>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Barycentric_coordinates_2/Triangle_coordinates_2.h>
#include <CGAL/Barycentric_coordinates_2/Maximum_entropy_2.h>
#include <CGAL/Barycentric_coordinates_2/Generalized_barycentric_coordinates_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;

typedef Kernel::FT         Scalar;
typedef Kernel::Point_2    Point;

typedef std::vector<Scalar> Coordinate_vector;
typedef std::vector<Point>  Point_vector;

typedef std::back_insert_iterator<Coordinate_vector> Vector_insert_iterator;

typedef CGAL::Barycentric_coordinates::Maximum_entropy_newton_solver<Kernel> MEC_newton_solver;
typedef CGAL::Barycentric_coordinates::Maximum_entropy_prior_function_type_one_2<Kernel> MEC1_prior;

typedef CGAL::Barycentric_coordinates::Triangle_coordinates_2<Kernel> Triangle_coordinates;
typedef CGAL::Barycentric_coordinates::Maximum_entropy_2<Kernel, MEC1_prior, MEC_newton_solver> Maximum_entropy;
typedef CGAL::Barycentric_coordinates::Generalized_barycentric_coordinates_2<Maximum_entropy, Kernel> Maximum_entropy_coordinates;

typedef boost::optional<Vector_insert_iterator> Output_type;

using std::cout; using std::endl; using std::string;

int main()
{
    const Point first_vertex  = Point(0.0, 0.0);
    const Point second_vertex = Point(1.0, 0.0);
    const Point third_vertex  = Point(0.0, 1.0);

    Triangle_coordinates triangle_coordinates(first_vertex, second_vertex, third_vertex);

    Point_vector vertices(3);
    vertices[0] = first_vertex; vertices[1] = second_vertex; vertices[2] = third_vertex;

    Maximum_entropy_coordinates maximum_entropy_coordinates(vertices.begin(), vertices.end());

    Coordinate_vector tri_coordinates;
    Coordinate_vector  me_coordinates;

    const Scalar step  = Scalar(1) / Scalar(100);
    const Scalar scale = Scalar(50);

    int count = 0;
    const Scalar limit = scale*step;

    for(Scalar x = step; x < limit; x += step) {
        for(Scalar y = step; y < limit; y += step) {
            const Point point(x, y);

            const Output_type tri_result = triangle_coordinates(point, tri_coordinates);
            const Output_type  dh_result = maximum_entropy_coordinates(point, me_coordinates);

            assert(tri_coordinates[count + 0] - me_coordinates[count + 0] <= Scalar(1e-5) &&
                   tri_coordinates[count + 1] - me_coordinates[count + 1] <= Scalar(1e-5) &&
                   tri_coordinates[count + 2] - me_coordinates[count + 2] <= Scalar(1e-5) );

            if( tri_coordinates[count + 0] - me_coordinates[count + 0] > Scalar(1e-5) ||
                tri_coordinates[count + 1] - me_coordinates[count + 1] > Scalar(1e-5) ||
                tri_coordinates[count + 2] - me_coordinates[count + 2] > Scalar(1e-5)  )
            {
                cout << endl << "MEC_triangle_inexact_test: FAILED." << endl << endl;
                exit(EXIT_FAILURE);
            }
            count += 3;
        }
    }

    cout << endl << "MEC_triangle_inexact_test: PASSED." << endl << endl;

    return EXIT_SUCCESS;
}
