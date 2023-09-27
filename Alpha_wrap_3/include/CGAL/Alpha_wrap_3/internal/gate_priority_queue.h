// Copyright (c) 2019-2022 Google LLC (USA).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Pierre Alliez
//                 Cedric Portaneri,
//                 Mael Rouxel-Labbé
//                 Andreas Fabri
//                 Michael Hemmer
//
#ifndef CGAL_ALPHA_WRAP_3_INTERNAL_GATE_PRIORITY_QUEUE_H
#define CGAL_ALPHA_WRAP_3_INTERNAL_GATE_PRIORITY_QUEUE_H

#include <CGAL/license/Alpha_wrap_3.h>

#include <boost/property_map/property_map.hpp>

#include <cassert>
#include <iostream>

namespace CGAL {
namespace Alpha_wraps_3 {
namespace internal {

// Represents an alpha-traversable facet in the mutable priority queue
template <typename Tr>
class Gate
{
  using Facet = typename Tr::Facet;
  using FT = typename Tr::Geom_traits::FT;

private:
  Facet m_facet;
  FT m_priority; // circumsphere sq_radius
  bool m_is_artificial_facet;

public:
  // Constructors
  Gate(const Facet& facet,
       const FT& priority,
       const bool is_artificial_facet)
    :
      m_facet(facet),
      m_priority(priority),
      m_is_artificial_facet(is_artificial_facet)
  {
    CGAL_assertion(priority >= 0);
  }

  // This overload is only used for contains() and erase(), priority and bbox flag are dummy value
  Gate(const Facet& facet)
    : Gate(facet, 0, false)
  { }

public:
  const Facet& facet() const { return m_facet; }
  const FT& priority() const { return m_priority; }
  bool is_artificial_facet() const { return m_is_artificial_facet; }
};

struct Less_gate
{
  template <typename Tr>
  bool operator()(const Gate<Tr>& a, const Gate<Tr>& b) const
  {
    return a.priority() > b.priority();
  }
};

template <typename Tr>
struct Gate_ID_PM
{
  using key_type = Gate<Tr>;
  using value_type = std::size_t;
  using reference = std::size_t;
  using category = boost::readable_property_map_tag;

  inline friend value_type get(Gate_ID_PM, const key_type& k)
  {
    using Facet = typename Tr::Facet;

    const Facet& f = k.facet();
    return (4 * f.first->time_stamp() + f.second);
  }
};

} // namespace internal
} // namespace Alpha_wraps_3
} // namespace CGAL

#endif // CGAL_ALPHA_WRAP_3_INTERNAL_GATE_PRIORITY_QUEUE_H
