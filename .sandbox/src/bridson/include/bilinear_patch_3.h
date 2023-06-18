// Copyright (c) 2000
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL$
// $Id$
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Jeffrey Cochran

#ifndef CGAL_CARTESIAN_BILINEAR_PATCH_3_H
#define CGAL_CARTESIAN_BILINEAR_PATCH_3_H

#include <CGAL/Handle_for.h>
#include <CGAL/array.h>

namespace CGAL {

template <class R_>
class BilinearPatchC3
{
  typedef typename R_::FT                   FT;
  typedef typename R_::Point_3              Point_3;
  typedef typename R_::Vector_3             Vector_3;
  typedef typename R_::Plane_3              Plane_3;
  typedef typename R_::Triangle_3           Triangle_3;

  typedef std::array<Point_3, 4>          Rep;
  typedef typename R_::template Handle<Rep>::type  Base;

  Base base;

public:
  typedef R_                                     R;

  BilinearPatchC3() {}

  BilinearPatchC3(const Point_3 &p, const Point_3 &q, const Point_3 &r, const Point_3 &s)
    : base(CGAL::make_array(p, q, r, s)) {}

  bool  operator==(const BilinearPatchC3 &bp) const;
//   bool  operator!=(const BilinearPatchC3 &bp) const;

  // bool  has_on(const Point_3 &p) const;
//   bool  is_degenerate() const;

//   const Point_3 & vertex(int i) const;
//   const Point_3 & operator[](int i) const;
//   const Tetrahedron_3 & tetrahedron() const;
};

template < class R >
bool
BilinearPatchC3<R>::operator==(const BilinearPatchC3<R> &bp) const
{
  if (CGAL::identical(base, bp.base))
      return true;

  int i;
  for(i=0; i<4; i++)
    if ( vertex(0) == bp.vertex(i) )
       break;

  // I assumed this test was to ensure that the set of vertices is not only the same, 
  // but that the orientation is the same as well
  return (i<4) && vertex(1) == bp.vertex(i+1) && vertex(2) == bp.vertex(i+2) && vertex(3) == bp.vertex(i+3);
}

// template < class R >
// inline
// bool
// BilinearPatchC3<R>::operator!=(const BilinearPatchC3<R> &bp) const
// {
//   return !(*this == bp);
// }

// template < class R >
// const typename BilinearPatchC3<R>::Point_3 &
// BilinearPatchC3<R>::vertex(int i) const
// {
//   if (i<0) i=(i%4)+4;
//   else if (i>3) i=i%4;
//   return (i==0) ? get_pointee_or_identity(base)[0] :
//          (i==1) ? get_pointee_or_identity(base)[1] :
//          (i==2) ? get_pointee_or_identity(base)[2] :
//                   get_pointee_or_identity(base)[3];
// }

// template < class R >
// inline
// const typename BilinearPatchC3<R>::Point_3 &
// BilinearPatchC3<R>::operator[](int i) const
// {
//   return vertex(i);
// }

// // template < class R >
// // inline
// // bool
// // BilinearPatchC3<R>::
// // has_on(const typename BilinearPatchC3<R>::Point_3 &p) const
// // {
// //   return R().has_on_3_object()
// //                (static_cast<const typename R::BilinearPatchC3&>(*this), p);
// // }

// template < class R >
// bool
// BilinearPatchC3<R>::is_degenerate() const
// {
//   return (collinear(vertex(0),vertex(1),vertex(2)) && collinear(vertex(1),vertex(2),vertex(3)));
// }

} //namespace CGAL

#endif // CGAL_CARTESIAN_BILINEAR_PATCH_3_H
