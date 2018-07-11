// Author: Dmitry Anisimov.
// In this test we compute discrete harmonic coordinates for ~11280 strictly interior points with respect to
// some irregular strictly convex polygon and check if they satisfy constant and linear precision properties.
// The chosen data type is exact.

// Does not work with inexact kernel. We get inconsistency when comparing difference with zero.

#include <cassert>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Barycentric_coordinates_2/Discrete_harmonic_2.h>
#include <CGAL/Barycentric_coordinates_2/Generalized_barycentric_coordinates_2.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;

typedef Kernel::FT      Scalar;
typedef Kernel::Point_2 Point;

typedef std::vector<Scalar> Coordinate_vector;
typedef std::vector<Point>  Point_vector;

typedef std::pair<Point, bool> Point_with_property;
typedef CGAL::First_of_pair_property_map<Point_with_property> Point_map;
typedef std::vector<Point_with_property> Input_range;


typedef std::back_insert_iterator<Coordinate_vector> Vector_insert_iterator;

typedef CGAL::Barycentric_coordinates::Discrete_harmonic_2<Kernel> Discrete_harmonic;
typedef CGAL::Barycentric_coordinates::Generalized_barycentric_coordinates_2<Discrete_harmonic, Input_range, Point_map, Kernel> Discrete_harmonic_coordinates;

typedef boost::optional<Vector_insert_iterator> Output_type;

using std::cout; using std::endl; using std::string;

int main()
{
    Point_vector vertices(6);

    vertices[0] = Point(0, 0);                                     vertices[1] = Point(1, 0);                                     vertices[2] = Point(Scalar(7) /Scalar(4), Scalar(3)/Scalar(4));
    vertices[3] = Point(Scalar(5)/Scalar(4), Scalar(3)/Scalar(2)); vertices[4] = Point(Scalar(1)/Scalar(4), Scalar(3)/Scalar(2)); vertices[5] = Point(Scalar(-1)/Scalar(2), Scalar(5)/Scalar(4));

    Input_range point_range(6);

    for(size_t i = 0; i < 6; ++i)
    {
        point_range[i]=Point_with_property(vertices[i],false);
    }

    Discrete_harmonic_coordinates discrete_harmonic_coordinates(point_range, Point_map());

    Coordinate_vector coordinates;

    const Scalar step    = Scalar(1)/Scalar(100);
    const Scalar x_scale = Scalar(100);
    const Scalar y_scale = Scalar(115);

    int count = 0;
    const Point zero(0, 0);
    const Scalar limit_x = x_scale*step;
    const Scalar limit_y = y_scale*step;

    for(Scalar x = step; x < limit_x; x += step) {
        for(Scalar y = step; y < limit_y; y += step) {
            const Point point(x, y);

            const Output_type result = discrete_harmonic_coordinates.compute(point, coordinates);

            const Scalar coordinate_sum = coordinates[count + 0] +
                                          coordinates[count + 1] +
                                          coordinates[count + 2] +
                                          coordinates[count + 3] +
                                          coordinates[count + 4] +
                                          coordinates[count + 5] ;

            const Point linear_combination( vertices[0].x()*coordinates[count + 0] +
                                            vertices[1].x()*coordinates[count + 1] +
                                            vertices[2].x()*coordinates[count + 2] +
                                            vertices[3].x()*coordinates[count + 3] +
                                            vertices[4].x()*coordinates[count + 4] +
                                            vertices[5].x()*coordinates[count + 5] ,
                                            vertices[0].y()*coordinates[count + 0] +
                                            vertices[1].y()*coordinates[count + 1] +
                                            vertices[2].y()*coordinates[count + 2] +
                                            vertices[3].y()*coordinates[count + 3] +
                                            vertices[4].y()*coordinates[count + 4] +
                                            vertices[5].y()*coordinates[count + 5] );

            const Point difference(linear_combination.x() - point.x(), linear_combination.y() - point.y());

            assert( (coordinate_sum == Scalar(1)) && (difference == zero) );

            if( (coordinate_sum != Scalar(1)) || (difference != zero) )
            {
                cout << endl << "DH_const_linear_precision_test: FAILED." << endl << endl;
                exit(EXIT_FAILURE);
            }
            count += 6;
        }
    }

    cout << endl << "DH_const_linear_precision_test: PASSED." << endl << endl;

    return EXIT_SUCCESS;
}
