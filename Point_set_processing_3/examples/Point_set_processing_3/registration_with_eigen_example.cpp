#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/draw_surface_mesh.h>
#include <CGAL/Aff_transformation_3.h>
#include <CGAL/Polygon_mesh_processing/transform.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Search_traits_adapter.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>

typedef CGAL::Simple_cartesian<double>                       Kernel;
typedef Kernel::Point_3                                      Point;
typedef Kernel::Vector_3                                     Vector;
typedef CGAL::Surface_mesh<Point>                            Mesh;
typedef CGAL::Aff_transformation_3<Kernel>                   Transform;

typedef boost::graph_traits<Mesh>::vertex_descriptor                                        vertex_descriptor;
typedef boost::associative_property_map< std::map<size_t, Point> >                     IndexMap;

typedef CGAL::Search_traits_3<Kernel>                                                       BaseTraits;
typedef CGAL::Search_traits_adapter<size_t, IndexMap, BaseTraits>                      Traits;
typedef CGAL::Orthogonal_k_neighbor_search<Traits>                                          Neighbor_search;
typedef Neighbor_search::Tree                                                               Tree;
typedef Tree::Splitter                                                                      Splitter;
typedef Neighbor_search::Distance                                                           Distance;


Mesh merge_meshes(Mesh a, Mesh b) {
    for (auto f : a.faces()) {
        std::array<Mesh::Vertex_index, 3> triangle;
        int i = 0;
        for (Mesh::Vertex_index v : a.vertices_around_face(a.halfedge(f))) {
            triangle[i] = b.add_vertex(a.point(v));
            ++i;
        }
        b.add_face(triangle[0], triangle[1], triangle[2]);
    }
    return b;
}


// w1: point-to-point weight
// w2: point-to-plane weight
// w3: rigid transformation weight
std::pair<Transform, Mesh> rigid_registration(const Mesh& source, const Mesh& target,
                                              double w1 = 0.1, double w2 = 0.1, double w3 = 1.0,
                                              int max_iterations = 100) {
    // resulting transformation is (R, t)
    Eigen::Matrix3d R_all = Eigen::Matrix3d::Identity();
    Eigen::Vector3d t_all = Eigen::Vector3d::Zero();
    size_t N = source.number_of_vertices();
    Mesh z = source;
    // build k-d tree for nearest neighbor search
    std::map<size_t, Point> index_map_base;
    for (auto it = target.vertices().begin(); it != target.vertices().end(); ++it) {
        index_map_base[it->idx()] = target.point(*it);
    }
    IndexMap index_map(index_map_base);
    Distance distance(index_map);
    Tree tree(boost::counting_iterator<size_t>(0),
              boost::counting_iterator<size_t>(N),
              Splitter(),
              Traits(index_map)); // when called, returns the index of the nearest neighbor on target mesh
    auto vertexnormals = target.property_map<vertex_descriptor, Vector>("v:normal").first;
    // solver
    Eigen::ConjugateGradient<Eigen::SparseMatrix<double, Eigen::RowMajor>, Eigen::Lower | Eigen::Upper> cg;
    cg.setMaxIterations(1000);
    cg.setTolerance(1e-6);
    // sparse coefficient matrix A
    Eigen::SparseMatrix<double, Eigen::RowMajor> A(6 + 3 * N, 6 + 3 * N);
    // column-wise sparsity pattern
    Eigen::VectorXd a = Eigen::VectorXd::Ones(6 + 3 * N);
    a.head(3) *= 3 + 2 + 2 * N;
    a.segment<3>(3) *= 2 + 1 + N;
    a.tail(3 * N) *= 2 + 1 + 3;
    A.reserve(a);
    // result vector b
    Eigen::VectorXd b(6 + 3 * N);
    double error = std::numeric_limits<double>::max();
    for (size_t iter = 0; iter < max_iterations; ++iter) {
        b.setZero();
        auto start = std::chrono::high_resolution_clock::now();
        for (auto it = source.vertices().begin(); it != source.vertices().end(); ++it) {
            auto i = it->idx();
            Point xp = source.point(*it);
            Eigen::Vector3d x(xp.x(), xp.y(), xp.z());
            // apply transformation to original x
            Eigen::Vector3d x_t = R_all * x + t_all;
            // search the nearest neighbor PI on the target mesh, n is the normal at PI
            Point zp = z.point(*it);
            size_t index = Neighbor_search(tree, zp, 1, 0, true, distance).begin()->first;
            Point pnn = index_map[index];
            Eigen::Vector3d PI(pnn.x(), pnn.y(), pnn.z());
            Vector nnn = vertexnormals[CGAL::SM_Vertex_index(index)];
            Eigen::Vector3d n(nnn.x(), nnn.y(), nnn.z());
            // build A and b
            Eigen::Matrix3d n_matrix = n * n.transpose();
            Eigen::Matrix3d z_block = (1 + w1 / w3) * Eigen::Matrix3d::Identity() + w2 / w3 * n_matrix;
            Eigen::Matrix3d X_t;
            X_t << 0, -x_t(2), x_t(1),
                   x_t(2), 0, -x_t(0),
                   -x_t(1), x_t(0), 0;
            Eigen::Matrix3d XX_t(X_t * X_t);
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    if (i == 0) {
                        A.coeffRef(j, k) = 0;
                        if (j != k) {
                            A.coeffRef(j, 3 + k) = 0; // A_tr
                            A.coeffRef(3 + j, k) = 0; // A_rt
                        } else {
                            A.coeffRef(3 + j, 3 + k) = 0; // A_tt
                        }
                    }
                    A.coeffRef(j, k) -= XX_t(j, k); // A_rr
                    if (j != k) {
                        A.coeffRef(j, 3 + k) += X_t(j, k); // A_tr
                        A.coeffRef(3 + j, k) -= X_t(j, k); // A_rt
                        A.coeffRef(j, 6 + 3 * i + k) = -X_t(j, k); // A_zir
                        A.coeffRef(6 + 3 * i + j, k) = X_t(j, k); // A_rzj
                    } else {
                        A.coeffRef(3 + j, 3 + k) += 1; // A_tt
                        A.coeffRef(3 + j, 6 + 3 * i + k) = -1; // A_zit
                        A.coeffRef(6 + 3 * i + j, 3 + k) = -1; // A_tzj
                    }
                    A.coeffRef(6 + 3 * i + j, 6 + 3 * i + k) = z_block(j, k); // A_zizj
                }
			}
            // b_r is zero
            b.segment<3>(3) -= x_t; // b_t
            b.segment<3>(6 + 3 * i) = (w1 / w3 * Eigen::Matrix3d::Identity() + w2 / w3 * n_matrix) * PI + x_t; // b_zi
        }
        auto stop = std::chrono::high_resolution_clock::now();
        std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;
        A.makeCompressed();
        cg.compute(A);
        Eigen::VectorXd x = cg.solve(b);
        if (cg.info() != Eigen::Success) {
            std::cout << "CG hasn't converged within " << cg.iterations() << " iterations. Error: " << cg.error() << "." << std::endl;
        }
        /*
        if (cg.info() == Eigen::Success) {
            std::cout << "Convergence achieved!" << std::endl;
        }
        std::cout << "#iterations:     " << cg.iterations() << std::endl;
        std::cout << "estimated error: " << cg.error() << std::endl;
        */
        // update r
        Eigen::Vector3d r(x[0], x[1], x[2]);
        auto roll = Eigen::AngleAxisd(x[0], Eigen::Vector3d::UnitX());
        auto pitch = Eigen::AngleAxisd(x[1], Eigen::Vector3d::UnitY());
        auto yaw = Eigen::AngleAxisd(x[2], Eigen::Vector3d::UnitZ());
        Eigen::Matrix3d R = (roll * pitch * yaw).matrix();
        R_all *= R;
        // update t
        Eigen::Vector3d t(x[3], x[4], x[5]);
        t_all += t;
        // update z
        for (auto it = z.vertices().begin(); it != z.vertices().end(); ++it) {
            size_t i = it->idx();
            Point p(x[6 + 3*i], x[6+ 3*i + 1], x[6 + 3*i+2]);
            z.point(*it) = p;
        }
        // calculate error
        double new_error = 0;
        for (auto it = source.vertices().begin(); it != source.vertices().end(); ++it) {
            Point xp = source.point(*it);
            Eigen::Vector3d x(xp.x(), xp.y(), xp.z());
            Point zp = z.point(*it);
            Eigen::Vector3d ref_z(zp.x(), zp.y(), zp.z());
            size_t index = Neighbor_search(tree, zp, 1, 0, true, distance).begin()->first;
            Point pnn = index_map[index];
            Eigen::Vector3d PI(pnn.x(), pnn.y(), pnn.z());
            Vector nnn = vertexnormals[CGAL::SM_Vertex_index(index)];
            Eigen::Vector3d n(nnn.x(), nnn.y(), nnn.z());
            // point-to-point error
            new_error += w1 * (PI - ref_z).squaredNorm();
            // point-to-plane error
            new_error += w2 * std::pow(n.dot(PI - ref_z), 2);
            // rigid transformation error
            new_error += w3 * (R_all * x + t_all - ref_z).squaredNorm();
        }
        std::cout << "Iteration: " << iter << " Error: " << new_error << " \n" << R_all << "\n" << t_all << std::endl;
        if (new_error < error) {
            error = new_error;
        } else {
            break;
        }
    }
    Transform transform(R_all(0, 0), R_all(0, 1), R_all(0, 2), t_all(0),
                        R_all(1, 0), R_all(1, 1), R_all(1, 2), t_all(1),
                        R_all(2, 0), R_all(2, 1), R_all(2, 2), t_all(2), 1);
    return std::make_pair(transform, z);
}


// w1: point-to-point
// w2: point-to-plane
// w3: rigid transformation
// w4: as-rigid-as-possible transformation
std::pair<Transform, Mesh> nonrigid_registration(const Mesh& source, const Mesh& target,
                                                 double w1 = 0.1, double w2 = 0.1, double w3 = 1.0, double w4 = 10.0,
                                                 int max_iter = 100) {
    size_t N = source.number_of_vertices();
    Mesh z = source;
    // build k-d tree for nearest neighbor search
    std::map<size_t, Point> index_map_base;
    for (auto it = target.vertices().begin(); it != target.vertices().end(); ++it) {
        index_map_base[it->idx()] = target.point(*it);
    }
    IndexMap index_map(index_map_base);
    Distance distance(index_map);
    Tree tree(boost::counting_iterator<size_t>(0),
        boost::counting_iterator<size_t>(N),
        Splitter(),
        Traits(index_map)); // when called, returns the index of the nearest neighbor on target mesh
    auto vertexnormals = target.property_map<vertex_descriptor, Vector>("v:normal").first;
    // solver
    Eigen::BiCGSTAB<Eigen::SparseMatrix<double, Eigen::RowMajor>> cg;
    cg.setMaxIterations(1000);
    cg.setTolerance(1e-6);
    // sparse coefficient matrix A
    Eigen::SparseMatrix<double, Eigen::RowMajor> A(6 + 2 * 3 * N, 6 + 2 * 3 * N);
    // column-wise sparsity pattern
    Eigen::VectorXd a = Eigen::VectorXd::Ones(6 + 2 * 3 * N);
    a.head(3) *= 3 + 2 + 2 * N;
    a.segment<3>(3) *= 2 + 1 + N;
    a.segment(6, 3 * N) *= 3 + 2;
    a.tail(3 * N) *= 2 + 1 + 2 + 3;
    for (auto e : z.edges()) {
        size_t v0 = CGAL::source(e, z);
        size_t v1 = CGAL::target(e, z);
        a.segment(6 + 3 * v0, 3) += 2 * Eigen::Vector3d::Ones();
        a.segment(6 + 3 * v1, 3) += 2 * Eigen::Vector3d::Ones();
        a.segment(6 + 3 * N + 3 * v0, 3) += (2 + 1) * Eigen::Vector3d::Ones();
        a.segment(6 + 3 * N + 3 * v1, 3) += (2 + 1) * Eigen::Vector3d::Ones();
    }
    A.reserve(a);
    // result vector b
    Eigen::VectorXd b(6 + 2 * 3 * N);
    double error = std::numeric_limits<double>::max();
    for (size_t iter = 0; iter < max_iter; ++iter) {
        b.setZero();
        auto start = std::chrono::high_resolution_clock::now();
        for (auto it = z.vertices().begin(); it != z.vertices().end(); ++it) {
            auto i = it->idx();
            Point zp = z.point(*it);
            Eigen::Vector3d x_t(zp.x(), zp.y(), zp.z());
            // search the nearest neighbor PI on the target mesh, n is the normal at PI
            size_t index = Neighbor_search(tree, zp, 1, 0, true, distance).begin()->first;
            Point pnn = index_map[index];
            Eigen::Vector3d PI(pnn.x(), pnn.y(), pnn.z());
            Vector nnn = vertexnormals[CGAL::SM_Vertex_index(index)];
            Eigen::Vector3d n(nnn.x(), nnn.y(), nnn.z());
            // build A and b
            Eigen::Matrix3d n_matrix = n * n.transpose();
            Eigen::Matrix3d z_diag_block = (w1 + w3) * Eigen::Matrix3d::Identity() + w2 * n_matrix;
            Eigen::Matrix3d X_t;
            X_t << 0, -x_t(2), x_t(1),
                   x_t(2), 0, -x_t(0),
                   -x_t(1), x_t(0), 0;
            Eigen::Matrix3d XX_t(X_t * X_t);
            Eigen::Matrix3d rirj_diag_block = Eigen::Matrix3d::Zero();
            Eigen::Matrix3d zirj_diag_block = Eigen::Matrix3d::Zero();
            Eigen::Matrix3d rizj_diag_block = Eigen::Matrix3d::Zero();
            for (auto he : CGAL::halfedges_around_target(*it, z)) {
                auto v0 = CGAL::source(he, z);
                Point xpk = z.point(v0);
                Eigen::Vector3d x_t_k(xpk.x(), xpk.y(), xpk.z());
                Eigen::Matrix3d X_t_k;
                X_t_k << 0, -x_t_k(2), x_t_k(1),
					     x_t_k(2), 0, -x_t_k(0),
					     -x_t_k(1), x_t_k(0), 0;
                rirj_diag_block -= (X_t_k - X_t) * (X_t_k - X_t);
                zirj_diag_block += X_t_k - X_t;
                rizj_diag_block -= w4 * (X_t_k - X_t);
                z_diag_block += 2 * w4 * Eigen::Matrix3d::Identity();
                for (int j = 0; j < 3; ++j) {
                    for (int k = 0; k < 3; ++k) {
                        if (j != k) {
                            A.coeffRef(6 + 3 * size_t(v0) + j,
                                       6 + 3 * N + 3 * i + k) = (X_t_k(j, k) - X_t(j, k)); // zirj off diag
                            A.coeffRef(6 + 3 * N + 3 * size_t(v0) + j,
                                       6 + 3 * i + k) = w4 * (X_t_k(j, k) - X_t(j, k)); // rizj off diag
                        } else {
                            A.coeffRef(6 + 3 * N + 3 * size_t(v0) + j,
                                       6 + 3 * N + 3 * i + k) = -2 * w4; // zizj off diag
                        }
                    }
                }
            }
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    if (i == 0) {
                        A.coeffRef(j, k) = 0;
                        if (j != k) {
                            A.coeffRef(j, 3 + k) = 0; // A_tr
                            A.coeffRef(3 + j, k) = 0; // A_rt
                        }
                        else {
                            A.coeffRef(3 + j, 3 + k) = 0; // A_tt
                        }
                    }
                    A.coeffRef(j, k) -= XX_t(j, k); // A_rr
                    if (j != k) {
                        A.coeffRef(j, 3 + k) += X_t(j, k); // A_tr
                        A.coeffRef(3 + j, k) -= X_t(j, k); // A_rt
                        A.coeffRef(j, 6 + 3 * N + 3 * i + k) = -X_t(j, k); // A_zir
                        A.coeffRef(6 + 3 * N + 3 * i + j, k) = w3 * X_t(j, k); // A_rzj
                        A.coeffRef(6 + 3 * i + j,
                                   6 + 3 * N + 3 * i + k) = zirj_diag_block(j, k); // A_zirj
                        A.coeffRef(6 + 3 * N + 3 * i + j,
                                   6 + 3 * i + k) = rizj_diag_block(j, k); // A_rizj
                    } else {
                        A.coeffRef(3 + j, 3 + k) += 1; // A_tt
                        A.coeffRef(3 + j, 6 + 3 * N + 3 * i + k) = -1; // A_zit
                        A.coeffRef(6 + 3 * N + 3 * i + j, 3 + k) = -w3; // A_tzj
                    }
                    A.coeffRef(6 + 3 * i + j, 6 + 3 * i + k) = rirj_diag_block(j, k); // A_rirj
                    A.coeffRef(6 + 3 * N + 3 * i + j, 6 + 3 * N + 3 * i + k) = z_diag_block(j, k); // A_zizj
                }
            }
            // b_r is zero
            b.segment<3>(3) -= x_t; // b_t
            // b_ri is zero
            b.segment<3>(6 + 3 * N + 3 * i) = (w1 * Eigen::Matrix3d::Identity() + w2 * n_matrix) * PI + w3 * x_t; // b_zi
            for (auto he : CGAL::halfedges_around_target(*it, z)) {
                auto v0 = CGAL::source(he, z);
                Point xpk = z.point(v0);
                Eigen::Vector3d x_t_k(xpk.x(), xpk.y(), xpk.z());
                b.segment<3>(6 + 3 * N + 3 * i) -= 2 * w4 * (x_t_k - x_t);
            }
        }
        auto stop = std::chrono::high_resolution_clock::now();
        std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;
        A.makeCompressed();
        cg.compute(A);
        Eigen::VectorXd solution = cg.solve(b);
        if (cg.info() != Eigen::Success) {
            std::cout << "CG hasn't converged within " << cg.iterations() << " iterations. Error: " << cg.error() << "." << std::endl;
        }
        /*
        if (cg.info() == Eigen::Success) {
            std::cout << "Convergence achieved!" << std::endl;
        }
        std::cout << "#iterations:     " << cg.iterations() << std::endl;
        std::cout << "estimated error: " << cg.error() << std::endl;
        */
        // update z
        for (auto it = z.vertices().begin(); it != z.vertices().end(); ++it) {
            auto i = it->idx();
            Point p(solution[6 + 3 * N + 3 * i], solution[6 + 3 * N + 3 * i + 1], solution[6 + 3 * N + 3 * i + 2]);
            z.point(*it) = p;
        }
        std::cout << "Iteration: " << iter << std::endl;
    }
    Transform transform(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1);
    return std::make_pair(transform, z);
}


int main(int argc, char* argv[]) {
    const std::string filename1 = argv[1]; //CGAL::data_file_path("meshes/wolf1.ply");
    Mesh mesh1;
    CGAL::IO::read_PLY(std::ifstream(filename1), mesh1);
    const std::string filename2 = argv[2]; //CGAL::data_file_path("meshes/wolf2.ply");
    Mesh mesh2;
    CGAL::IO::read_PLY(std::ifstream(filename2), mesh2);
    CGAL::draw(merge_meshes(mesh1, mesh2));
    auto result = rigid_registration(mesh1, mesh2, 0.0, 0.1, 1.0, 10);
    Transform transform = result.first;
    Mesh z = result.second;
    CGAL::Polygon_mesh_processing::transform(transform, mesh1);
    //CGAL::Polygon_mesh_processing::transform(Transform(CGAL::Translation(), Vector(-40, 0, 0)), mesh1);
    //CGAL::draw(merge_meshes(mesh1, mesh2));
    result = nonrigid_registration(mesh1, mesh2, 0.0, 0.1, 1.0, 10.0, 10);
    transform = result.first;
    z = result.second;
    CGAL::Polygon_mesh_processing::transform(Transform(CGAL::Translation(), Vector(-40, 0, 0)), z);
    CGAL::draw(merge_meshes(z, mesh2));
    return EXIT_SUCCESS;
}
