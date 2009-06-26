// Copyright (c) 2009 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: $
// $Id: $
//
//
// Author(s)     : Pierre Alliez, Stephane Tayeb, Camille Wormser
//
//******************************************************************************
// File Description :
//
//******************************************************************************

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Simple_cartesian.h>

#include <CGAL/AABB_polyhedron_triangle_primitive.h>
#include <CGAL/AABB_polyhedron_segment_primitive.h>


double random_in(const double a,
                 const double b)
{
    double r = rand() / (double)RAND_MAX;
    return a + (b - a) * r;
}

template <class K>
typename K::Point_3 random_point_in(const CGAL::Bbox_3& bbox)
{
    typedef typename K::FT FT;
    FT x = (FT)random_in(bbox.xmin(),bbox.xmax());
    FT y = (FT)random_in(bbox.ymin(),bbox.ymax());
    FT z = (FT)random_in(bbox.zmin(),bbox.zmax());
    return typename K::Point_3(x,y,z);
}

template <class K>
typename K::Vector_3 random_vector()
{
    typedef typename K::FT FT;
    FT x = (FT)random_in(0.0,1.0);
    FT y = (FT)random_in(0.0,1.0);
    FT z = (FT)random_in(0.0,1.0);
    return typename K::Vector_3(x,y,z);
}


template <class Tree, class K>
void test_all_intersection_query_types(Tree& tree)
{
    std::cout << "Test all query types" << std::endl;

    typedef typename K::FT FT;
    typedef typename K::Ray_3 Ray;
    typedef typename K::Line_3 Line;
    typedef typename K::Point_3 Point;
    typedef typename K::Vector_3 Vector;
    typedef typename K::Segment_3 Segment;
    typedef typename Tree::Primitive Primitive;
    typedef typename Tree::Point_and_primitive_id Point_and_primitive_id;
    typedef typename Tree::Object_and_primitive_id Object_and_primitive_id;

    Point p((FT)-0.5, (FT)-0.5, (FT)-0.5);
    Point q((FT) 0.5, (FT) 0.5, (FT) 0.5);
    Ray ray(p,q);
    Line line(p,q);
    Segment segment(p,q);
    bool success = false;

    // do_intersect
    success = tree.do_intersect(ray);
    success = tree.do_intersect(line);
    success = tree.do_intersect(segment);

    // number_of_intersected_primitives
    tree.number_of_intersected_primitives(ray);
    tree.number_of_intersected_primitives(line);
    tree.number_of_intersected_primitives(segment);

    // all_intersected_primitives
    std::list<typename Primitive::Id> primitives;
    tree.all_intersected_primitives(ray,std::back_inserter(primitives));
    tree.all_intersected_primitives(line,std::back_inserter(primitives));
    tree.all_intersected_primitives(segment,std::back_inserter(primitives));

    // any_intersection
    boost::optional<Object_and_primitive_id> optional_object_and_primitive;
    optional_object_and_primitive = tree.any_intersection(ray);
    optional_object_and_primitive = tree.any_intersection(line);
    optional_object_and_primitive = tree.any_intersection(segment);

    // any_intersected_primitive
    boost::optional<typename Primitive::Id> optional_primitive;
    optional_primitive = tree.any_intersected_primitive(ray);
    optional_primitive = tree.any_intersected_primitive(line);
    optional_primitive = tree.any_intersected_primitive(segment);

    // all_intersections
    std::list<Object_and_primitive_id> intersections;
    tree.all_intersections(ray,std::back_inserter(intersections));
    tree.all_intersections(line,std::back_inserter(intersections));
    tree.all_intersections(segment,std::back_inserter(intersections));
}


template <class Tree, class K>
void test_all_distance_query_types(Tree& tree)
{
    typedef typename K::FT FT;
    typedef typename K::Ray_3 Ray;
    typedef typename K::Point_3 Point;
    typedef typename K::Vector_3 Vector;
    typedef typename Tree::Primitive Primitive;
    typedef typename Tree::Point_and_primitive_id Point_and_primitive_id;

    Point query = random_point_in<K>(tree.bbox());
    Point_and_primitive_id hint = tree.any_reference_point_and_id();

    FT sqd1 = tree.squared_distance(query);
    FT sqd2 = tree.squared_distance(query,hint.first);
    if(sqd1 != sqd2)
        std::cout << "warning: different distances with and without hint";

    Point p1 = tree.closest_point(query);
    Point p2 = tree.closest_point(query,hint.first);
    if(p1 != p2)
        std::cout << "Different closest points with and without hint (possible, in case there are more than one)";

    Point_and_primitive_id pp1 = tree.closest_point_and_primitive(query);
    Point_and_primitive_id pp2 = tree.closest_point_and_primitive(query,hint);
    if(pp1.second != pp2.second)
        std::cout << "Different closest primitives with and without hint (possible, in case there are more than one)";
}


template <class Tree, class K>
void test_distance_speed(Tree& tree,
                         const double duration)
{
    typedef typename K::FT FT;
    typedef typename K::Ray_3 Ray;
    typedef typename K::Point_3 Point;
    typedef typename K::Vector_3 Vector;

    CGAL::Timer timer;
    timer.start();
    unsigned int nb = 0;
    while(timer.time() < duration)
    {
            // picks a random point in the tree bbox
            Point query = random_point_in<K>(tree.bbox());
            Point closest = tree.closest_point(query);
            nb++;
    }
    double speed = (double)nb / timer.time();
    std::cout << speed << " distance queries/s" << std::endl;
    timer.stop();
}


//-------------------------------------------------------
// Helpers
//-------------------------------------------------------
enum Primitive_type {
  SEGMENT, TRIANGLE
};

/**
 * Primitive_generator : designed to tell void test<K,Primitive>(const char* filename)
 * some information about which primitive to use.
 *
 * Must define:
 *  type Primitive
 *  type iterator
 *  iterator begin(Polyhedron&)
 *  iterator end(Polyhedron&)
 *
 * begin & end are used to build the AABB_tree.
 */
template<Primitive_type Primitive, class K, class Polyhedron>
struct Primitive_generator {};

template<class K, class Polyhedron>
struct Primitive_generator<SEGMENT, K, Polyhedron>
{
    typedef CGAL::AABB_polyhedron_segment_primitive<K,Polyhedron> Primitive;

    typedef typename Polyhedron::Edge_iterator iterator;
    iterator begin(Polyhedron& p) { return p.edges_begin(); }
    iterator end(Polyhedron& p) { return p.edges_end(); }
};

template<class K, class Polyhedron>
struct Primitive_generator<TRIANGLE, K, Polyhedron>
{
    typedef CGAL::AABB_polyhedron_triangle_primitive<K,Polyhedron> Primitive;

    typedef typename Polyhedron::Facet_iterator iterator;
    iterator begin(Polyhedron& p) { return p.facets_begin(); }
    iterator end(Polyhedron& p) { return p.facets_end(); }
};



/**
 * Declaration only, implementation should be given in .cpp file
 */
template<class K, class Tree, class Polyhedron, Primitive_type Type>
void test_impl(Tree& tree, Polyhedron& p, const double duration);


/**
 * Generic test method. Build AABB_tree and call test_impl()
 */
template <class K, Primitive_type Primitive>
void test(const char *filename,
          const double duration)
{
    typedef CGAL::Polyhedron_3<K> Polyhedron;
    typedef Primitive_generator<Primitive,K,Polyhedron> Pr_generator;
    typedef typename Pr_generator::Primitive Pr;
    typedef CGAL::AABB_traits<K, Pr> Traits;
    typedef CGAL::AABB_tree<Traits> Tree;

    Polyhedron polyhedron;
    std::ifstream ifs(filename);
    ifs >> polyhedron;

    // constructs AABB tree and internal search KD-tree with
    // the points of the polyhedron
    Tree tree(Pr_generator().begin(polyhedron),Pr_generator().end(polyhedron));
    //tree.accelerate_distance_queries(polyhedron.points_begin(),polyhedron.points_end());

    // call all tests
    test_impl<K,Tree,Polyhedron,Primitive>(tree,polyhedron,duration);
}


/**
 * Generic test_kernel method. call test<K> for various kernel K.
 */
template<Primitive_type Primitive>
void test_kernels(const char *filename,
                  const double duration)
{
    std::cout << std::endl;
    std::cout << "Polyhedron " << filename << std::endl;
    std::cout << "============================" << std::endl;

    std::cout << std::endl;
    std::cout << "Simple cartesian float kernel" << std::endl;
    test<CGAL::Simple_cartesian<float>,Primitive>(filename,duration);

    std::cout << std::endl;
    std::cout << "Cartesian float kernel" << std::endl;
    test<CGAL::Cartesian<float>,Primitive>(filename,duration);

    std::cout << std::endl;
    std::cout << "Simple cartesian double kernel" << std::endl;
    test<CGAL::Simple_cartesian<double>,Primitive>(filename,duration);

    std::cout << std::endl;
    std::cout << "Cartesian double kernel" << std::endl;
    test<CGAL::Cartesian<double>,Primitive>(filename,duration);

    std::cout << std::endl;
    std::cout << "Epic kernel" << std::endl;
    test<CGAL::Exact_predicates_inexact_constructions_kernel,Primitive>(filename,duration);
}







//-------------------------------------------------------
// Naive Implementations
//-------------------------------------------------------

/**
 * Implements queries defined by AABB_tree class in naive way: iterate on
 * the primitive.
 */
template<typename Polyhedron,
         typename K,
         Primitive_type Primitive >
class Naive_implementations
{
  typedef Primitive_generator<Primitive,K,Polyhedron> Pr_generator;
  typedef typename Pr_generator::Primitive Pr;
  typedef CGAL::AABB_traits<K, Pr> Traits;
  typedef typename Pr_generator::iterator Polyhedron_primitive_iterator;
  typedef typename Traits::size_type size_type;
  typedef typename Traits::Object_and_primitive_id Object_and_primitive_id;
  typedef typename Pr::Id Primitive_id;
  typedef typename Traits::FT FT;
  typedef typename Traits::Point Point;
  typedef typename Traits::Point_and_primitive_id Point_and_primitive_id;

  typedef boost::optional<Object_and_primitive_id> Intersection_result;

public:
  template<typename Query>
  bool do_intersect(const Query& query, Polyhedron& p) const
  {
    Polyhedron_primitive_iterator it = Pr_generator().begin(p);
    for ( ; it != Pr_generator().end(p) ; ++it )
    {
      if ( Traits().do_intersect_object()(query, Pr(it) ) )
        return true;
    }

    return false;
  }

  template<typename Query>
  size_type number_of_intersected_primitives(const Query& query,
                                             Polyhedron& p) const
  {
    size_type result = 0;

    Polyhedron_primitive_iterator it = Pr_generator().begin(p);
    for ( ; it != Pr_generator().end(p) ; ++it )
    {
      if ( Traits().do_intersect_object()(query, Pr(it) ) )
        ++result;
    }

    return result;
  }

  template<typename Query, typename OutputIterator>
  OutputIterator all_intersected_primitives(const Query& query,
                                            Polyhedron& p,
                                            OutputIterator out) const
  {
    Polyhedron_primitive_iterator it = Pr_generator().begin(p);
    for ( ; it != Pr_generator().end(p) ; ++it )
    {
      if ( Traits().do_intersect_object()(query, Pr(it) ) )
        *out++ = Pr(it).id();
    }

    return out;
  }

  template<typename Query, typename OutputIterator>
  OutputIterator all_intersections(const Query& query,
                                   Polyhedron& p,
                                   OutputIterator out) const
  {
    Polyhedron_primitive_iterator it = Pr_generator().begin(p);
    for ( ; it != Pr_generator().end(p) ; ++it )
    {
      Intersection_result intersection  = Traits().intersection_object()(query, Pr(it));
      if ( intersection )
        *out++ = *intersection;
    }

    return out;
  }

  Point closest_point(const Point& query,
                      Polyhedron& p) const
  {
    Polyhedron_primitive_iterator it = Pr_generator().begin(p);

    assert ( it != Pr_generator().end(p) );

    // Get a point on the primitive
    Point closest_point = Pr(it).reference_point();

    for ( ; it != Pr_generator().end(p) ; ++it )
    {
      closest_point = Traits().closest_point_object()(query, Pr(it), closest_point);
    }

    return closest_point;
  }

  Point_and_primitive_id closest_point_and_primitive(const Point& query,
                                                     Polyhedron& p) const
  {
    Polyhedron_primitive_iterator it = Pr_generator().begin(p);

    assert ( it != Pr_generator().end(p) );

    // Get a point on the primitive
    Pr closest_primitive = Pr(it);
    Point closest_point = closest_primitive.reference_point();

    for ( ; it != Pr_generator().end(p) ; ++it )
    {
      Pr tmp_pr(it);
      Point tmp_pt = Traits().closest_point_object()(query, tmp_pr, closest_point);
      if ( tmp_pt != closest_point )
      {
        closest_point = tmp_pt;
        closest_primitive = tmp_pr;
      }
    }

    return Point_and_primitive_id(closest_point,closest_primitive.id());
  }
};


//-------------------------------------------------------
// Naive Tester
//-------------------------------------------------------
template <class Tree, class Polyhedron, class K, Primitive_type Type>
class Tree_vs_naive
{
  typedef typename K::FT FT;
  typedef typename K::Ray_3 Ray;
  typedef typename K::Line_3 Line;
  typedef typename K::Point_3 Point;
  typedef typename K::Vector_3 Vector;
  typedef typename K::Segment_3 Segment;

  typedef typename Tree::Primitive Primitive;
  typedef typename Tree::Point_and_primitive_id Point_and_primitive_id;
  typedef typename Tree::Object_and_primitive_id Object_and_primitive_id;
  typedef typename Tree::size_type size_type;

  typedef Naive_implementations<Polyhedron, K, Type> Naive_implementation;

public:
  Tree_vs_naive(Tree& tree, Polyhedron& p)
    : m_tree(tree)
    , m_polyhedron(p)
    , m_naive()
    , m_naive_time(0)
    , m_tree_time(0)    {}

  void test_all_intersection_methods(double duration) const
  {
    m_naive_time = 0;
    m_tree_time = 0;

    test_do_intersect(duration);
    test_number_of_intersected_primitives(duration);
    test_intersected_primitives(duration);
    test_intersections(duration);

    std::cerr << "\tNaive test time: " << m_naive_time*1000 << "ms" << std::endl;
    std::cerr << "\tTree test time: " << m_tree_time*1000 << "ms" << std::endl;
  }

  void test_all_distance_methods(double duration) const
  {
    m_naive_time = 0;
    m_tree_time = 0;

    test_closest_point(duration);
    test_closest_point_and_primitive(duration);

    std::cerr << "\tNaive test time: " << m_naive_time*1000 << "ms" << std::endl;
    std::cerr << "\tTree test time: " << m_tree_time*1000 << "ms" << std::endl;
  }

  void test_do_intersect(double duration) const
  {
    loop(duration, Do_intersect());
  }

  void test_number_of_intersected_primitives(double duration) const
  {
    loop(duration, Number_of_intersected_primitives());
  }

  void test_intersected_primitives(double duration) const
  {
    loop(duration, Intersected_primitives());
  }

  void test_intersections(double duration) const
  {
    loop(duration, Intersections());
  }

  void test_closest_point(double duration) const
  {
    loop_distance(duration, Closest_point());
  }

  void test_closest_point_and_primitive(double duration) const
  {
    loop_distance(duration, Closest_point_and_primitive());
  }


private:

  template<typename Test>
  void loop(double duration,
            const Test& test) const
  {
    CGAL::Timer timer;
    timer.start();
    int nb_test = 0;
    while ( timer.time() < duration )
    {
      Point a = random_point_in<K>(m_tree.bbox());
      Point b = random_point_in<K>(m_tree.bbox());
      Segment segment(a,b);
      Ray ray(a,b);
      Line line(a,b);

      test(segment, m_polyhedron, m_tree, m_naive);
      test(ray, m_polyhedron, m_tree, m_naive);
      test(line, m_polyhedron, m_tree, m_naive);

      ++nb_test;
    }
    timer.stop();

//    std::cerr << "\t" << nb_test << " loops in "
//              << timer.time() << "s" << std::endl;
    m_naive_time += test.naive_timer.time();
    m_tree_time += test.tree_timer.time();
  }

  template<typename Test>
  void loop_distance(double duration,
                     const Test& test) const
  {
    CGAL::Timer timer;
    timer.start();
    while ( timer.time() < duration )
    {
      Point a = random_point_in<K>(m_tree.bbox());
      test(a, m_polyhedron, m_tree, m_naive);
    }
    timer.stop();

    m_naive_time += test.naive_timer.time();
    m_tree_time += test.tree_timer.time();
  }

private:
  /**
   * Tests do_intersect
   */
  struct Do_intersect
  {
    template<typename Query>
    void
    operator()(const Query& query,
               Polyhedron& p,
               Tree& tree,
               const Naive_implementation& naive) const
    {
      naive_timer.start();
      bool result_naive = naive.do_intersect(query, p);
      naive_timer.stop();

      tree_timer.start();
      bool result_tree = tree.do_intersect(query);
      tree_timer.stop();

      // Check
      assert ( result_naive == result_tree );
    }

    mutable CGAL::Timer naive_timer;
    mutable CGAL::Timer tree_timer;
  };

  /**
   * Tests number_of_intersected_primitives
   */
  struct Number_of_intersected_primitives
  {
    template<typename Query>
    void
    operator()(const Query& query,
               Polyhedron& p,
               Tree& tree,
               const Naive_implementation& naive) const
    {
      naive_timer.start();
      size_type number_naive = naive.number_of_intersected_primitives(query, p);
      naive_timer.stop();

      tree_timer.start();
      size_type number_tree = tree.number_of_intersected_primitives(query);
      tree_timer.stop();

      // Check
      assert ( number_naive == number_tree );
    }

    mutable CGAL::Timer naive_timer;
    mutable CGAL::Timer tree_timer;
  };

  /**
   * Tests all_intersected_primitives and any_intersected_primitives
   */
  struct Intersected_primitives
  {
    template<typename Query>
    void
    operator()(const Query& query,
               Polyhedron& p,
               Tree& tree,
               const Naive_implementation& naive) const
    {
      typedef std::vector<typename Primitive::Id> Id_vector;

      Id_vector primitives_naive;
      naive_timer.start();
      naive.all_intersected_primitives(query, p, std::back_inserter(primitives_naive));
      naive_timer.stop();

      Id_vector primitives_tree;
      tree_timer.start();
      tree.all_intersected_primitives(query, std::back_inserter(primitives_tree));
      tree_timer.stop();

      // Check: warning, we don't know elements order...
      for ( typename Id_vector::iterator it = primitives_naive.begin() ;
            it != primitives_naive.end() ;
            ++it )
      {
        assert( std::find(primitives_tree.begin(), primitives_tree.end(), *it)
                != primitives_tree.end() );
      }

      // any_intersected_primitive test (do not count time here)
      typedef boost::optional<typename Primitive::Id> Any_primitive;
      Any_primitive primitive = tree.any_intersected_primitive(query);

      // Check: verify we do get the result by naive method
      if ( primitive )
      {
        assert( std::find(primitives_naive.begin(),
                          primitives_naive.end(),
                          *primitive)
               != primitives_naive.end());
      }
      else if ( primitives_naive.size() != 0 )
        assert(false);
    }

    mutable CGAL::Timer naive_timer;
    mutable CGAL::Timer tree_timer;
  };

  /**
   * Tests all_intersections and any_intersection
   */
  struct Intersections
  {
    template<typename Query>
    void
    operator()(const Query& query,
               Polyhedron& p,
               Tree& tree,
               const Naive_implementation& naive) const
    {
      typedef std::vector<Object_and_primitive_id> Obj_Id_vector;

      Obj_Id_vector intersections_naive;
      naive_timer.start();
      naive.all_intersections(query, p, std::back_inserter(intersections_naive));
      naive_timer.stop();

      Obj_Id_vector intersections_tree;
      tree_timer.start();
      tree.all_intersections(query, std::back_inserter(intersections_tree));
      tree_timer.stop();

      // Check: warning, we don't know elements order...
      // Test equality of vectors on ids only
      typedef std::vector<typename Primitive::Id> Id_vector;
      Id_vector intersections_naive_id;
      std::transform(intersections_naive.begin(),
                     intersections_naive.end(),
                     std::back_inserter(intersections_naive_id),
                     primitive_id);

      for ( typename Obj_Id_vector::iterator it = intersections_tree.begin() ;
            it != intersections_tree.end() ;
            ++it )
      {
        assert( std::find(intersections_naive_id.begin(),
                          intersections_naive_id.end(),
                          it->second)
                != intersections_naive_id.end() );
      }

      // Any intersection test (do not count time here)
      typedef boost::optional<Object_and_primitive_id> Any_intersection;
      Any_intersection intersection = tree.any_intersection(query);

      // Check: verify we do get the result by naive method
      if ( intersection )
      {
        assert( std::find(intersections_naive_id.begin(),
                          intersections_naive_id.end(),
                          intersection->second)
               != intersections_naive_id.end());
      }
      else if ( intersections_naive.size() != 0 )
        assert(false);

    }

    static typename Primitive::Id primitive_id(const Object_and_primitive_id& o)
    {
      return o.second;
    }

    mutable CGAL::Timer naive_timer;
    mutable CGAL::Timer tree_timer;
  };


  struct Closest_point
  {
    void operator()(const Point& query,
                    Polyhedron& p,
                    Tree& tree,
                    const Naive_implementation& naive) const
    {
      naive_timer.start();
      Point point_naive = naive.closest_point(query,p);
      naive_timer.stop();

      tree_timer.start();
      Point point_tree = tree.closest_point(query);
      tree_timer.stop();

      FT dist_naive = CGAL::squared_distance(query, point_naive);
      FT dist_tree = CGAL::squared_distance(query, point_tree);

      //assert( dist_tree >= dist_naive );
      const FT epsilon = (FT)1e-6;
      assert( (dist_naive - dist_tree) <= (epsilon * dist_tree) );
      assert( (dist_naive - dist_tree) >= (-1. * epsilon * dist_tree) );
    }

    mutable CGAL::Timer naive_timer;
    mutable CGAL::Timer tree_timer;
  };

  struct Closest_point_and_primitive
  {
    void operator()(const Point& query,
                    Polyhedron& p,
                    Tree& tree,
                    const Naive_implementation& naive) const
    {
      naive_timer.start();
      Point_and_primitive_id point_naive = naive.closest_point_and_primitive(query,p);
      naive_timer.stop();

      tree_timer.start();
      Point_and_primitive_id point_tree = tree.closest_point_and_primitive(query);
      tree_timer.stop();

      if ( point_naive.second == point_tree.second )
      {
        // Points should be the same
        assert(point_naive.first == point_tree.first);
      }
      else
      {
        // Compare distance
        FT dist_naive = CGAL::squared_distance(query, point_naive.first);
        FT dist_tree = CGAL::squared_distance(query, point_tree.first);

        //assert( dist_tree >= dist_naive );
        const FT epsilon = (FT)1e-6;
        assert( (dist_naive - dist_tree) <= (epsilon * dist_tree) );
        assert( (dist_naive - dist_tree) >= (-1 * epsilon * dist_tree) );
      }
    }

    mutable CGAL::Timer naive_timer;
    mutable CGAL::Timer tree_timer;
  };


private:
  Tree& m_tree;
  Polyhedron& m_polyhedron;
  Naive_implementation m_naive;
  mutable double m_naive_time;
  mutable double m_tree_time;
};
