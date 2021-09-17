// Copyright (c) 2021  GeometryFactory Sarl
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Andreas Fabri

#ifndef CGAL_INDEXED_TRIANGLE_SET_H
#define CGAL_INDEXED_TRIANGLE_SET_H

#include <CGAL/license/Convex_hull_3.h>

#include <vector>
#include <fstream>
#include <array>

namespace CGAL {

template <typename P>
struct Indexed_triangle_set
{
  std::vector<P> vertices;
  std::vector<std::array<int,3> > faces;
};


template <class P>
void make_tetrahedron(const P& p0, const P&p1, const P& p2, const P& p3,
                      Indexed_triangle_set<P>& its)
{
  std::cout << "make_tetrahedron" << std::endl;
}

template <class P>
void clear(Indexed_triangle_set<P>& its)
{
  its.vertices.clear();
  its.faces.clear();
}


template <class P>
std::ostream& operator<<(std::ostream& os, const Indexed_triangle_set<P>& its)
{
  os << "OFF\n";
  os << its.vertices.size() << " " << its.faces.size() << " \n";
  for(const P& p : its.vertices){
    os << p << "\n";
  }
  for(const std::array<int,3> f : its.faces){
    os << f[0] << " "<< f[1] << " "<< f[2] << "\n";
  }
  std::cout << std::flush;
  return os;
}



namespace Convex_hull_3 {
namespace internal {

template <class P>
void add_isolated_points(const P& point, Indexed_triangle_set<P>& its)
{
  std::cout << "add_isolated_points()" << std::endl;
}


template <typename P>
void copy_ch2_to_face_graph(const std::list<P>& CH_2,
                            Indexed_triangle_set<P>& its)
{
  std::cout << "copy_ch2_to_face_graph" << std::endl;
}


template <typename TDS, typename P>
void copy_face_graph(const TDS& tds, Indexed_triangle_set<P>& its)
{
  typedef typename TDS::Vertex_iterator Vertex_iterator;
  typedef typename TDS::Face_iterator Face_iterator;
  int i = 0;
  its.vertices.reserve(tds.number_of_vertices());
  its.faces.reserve(tds.number_of_faces());
  for(Vertex_iterator vit = tds.vertices_begin(); vit != tds.vertices_end(); ++vit){
      its.vertices.push_back(vit->point());
    vit->info() = i++;
  }
  for (Face_iterator fit = tds.faces_begin(); fit != tds.faces_end(); ++fit) {
      its.faces.push_back(CGAL::make_array(fit->vertex(0)->info(), fit->vertex(1)->info(), fit->vertex(2)->info()));
  }
}

}
}

} // namespace

#endif
