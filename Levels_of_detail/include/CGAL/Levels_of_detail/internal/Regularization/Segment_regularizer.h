// Copyright (c) 2019 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is a part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
//
// Author(s)     : Dmitry Anisimov, Simon Giraudot, Pierre Alliez, Florent Lafarge, and Andreas Fabri
//

#ifndef CGAL_LEVELS_OF_DETAIL_INTERNAL_SEGMENT_REGULARIZER_H
#define CGAL_LEVELS_OF_DETAIL_INTERNAL_SEGMENT_REGULARIZER_H

#include <CGAL/license/Levels_of_detail.h>

// STL includes.
#include <vector>
#include <utility>
#include <algorithm>

// CGAL includes.
#include <CGAL/assertions.h>
#include <CGAL/property_map.h>

// Internal includes.
#include <CGAL/Levels_of_detail/internal/utils.h>
#include <CGAL/Levels_of_detail/internal/struct.h>

// Spatial search.
#include <CGAL/Levels_of_detail/internal/Spatial_search/K_neighbor_query.h>

namespace CGAL {
namespace Levels_of_detail {
namespace internal {

  template<typename GeomTraits>
  class Segment_regularizer {

  public:
    using Traits = GeomTraits;

    using FT = typename Traits::FT;
    using Point_2 = typename Traits::Point_2;
    using Segment_2 = typename Traits::Segment_2;
    using Vector_2 = typename Traits::Vector_2;
    using Line_2 = typename Traits::Line_2;

    using FT_pair = std::pair<FT, FT>;
    using Size_pair = std::pair<std::size_t, std::size_t>;
    using Indices = std::vector<std::size_t>;
    using Seg_pair = std::pair<Segment_2, bool>;

    using Point_pair = std::pair<Point_2, std::size_t>;
    using Point_map = CGAL::First_of_pair_property_map<Point_pair>;
    using K_neighbor_query =
    internal::K_neighbor_query<Traits, std::vector<Point_pair>, Point_map>;

    Segment_regularizer(
      const FT min_length,
      const FT angle_bound,
      const FT ordinate_bound) :
    m_min_length(min_length),
    m_angle_bound(angle_bound),
    m_ordinate_bound(ordinate_bound),
    m_pi(static_cast<FT>(CGAL_PI)),
    m_angle_threshold(FT(5)),
    m_bound_min(m_angle_bound / FT(3)),
    m_bound_max(FT(90) - m_bound_min)
    { }

    void compute_multiple_directions(
      const std::vector<Segment_2>& segments_outer,
      const std::vector< std::vector<Segment_2> >& contours) {
      
      std::vector< std::vector<Seg_pair> > contours_outer;
      create_contours_from_segments(segments_outer, contours_outer);

      std::vector<FT_pair> bounds_outer;
      std::vector<Segment_2> longest_outer;
      std::vector<Indices> groups_outer;
      get_multiple_directions(
        segments_outer, contours_outer, bounds_outer, longest_outer, groups_outer);
      unify_along_contours(contours_outer, groups_outer);

      std::vector<Segment_2> segments_inner;
      for (const auto& contour : contours)
        for (const auto& segment : contour)
          segments_inner.push_back(segment);

      std::vector< std::vector<Seg_pair> > contours_inner;
      create_contours_from_segments(segments_inner, contours_inner);

      std::vector<FT_pair> bounds_inner;
      std::vector<Segment_2> longest_inner;
      std::vector<Indices> groups_inner;
      get_multiple_directions(
        segments_inner, contours_inner, bounds_inner, longest_inner, groups_inner);
      unify_along_contours(contours_inner, groups_inner);
      
      std::cout << "Num outer directions: " << longest_outer.size() << std::endl;
      std::cout << "Num inner directions: " << longest_inner.size() << std::endl;
      
      Point_map point_map; std::vector<Point_pair> pair_range;
      for (std::size_t k = 0; k < contours_outer.size(); ++k) {
        for (std::size_t i = 0; i < contours_outer[k].size(); ++i) {
          const auto& segment = contours_outer[k][i].first;
          
          const auto& s = segment.source();
          const auto& t = segment.target();

          const FT x = (s.x() + t.x()) / FT(2);
          const FT y = (s.y() + t.y()) / FT(2);

          const Point_2 p1 = s;
          const Point_2 p2 = Point_2(x, y);

          const std::size_t gr_idx = groups_outer[k][i];
          pair_range.push_back(std::make_pair(p1, gr_idx));
          pair_range.push_back(std::make_pair(p2, gr_idx));
        }
      }

      Indices neighbors;
      K_neighbor_query neighbor_query(pair_range, FT(1), point_map);

      m_bounds.clear();
      m_bounds = bounds_outer;

      m_longest.clear();
      m_longest = longest_outer;

      m_groups.clear();
      make_default_groups(contours, std::size_t(-1), m_groups);

      for (std::size_t k = 0; k < contours.size(); ++k) {
        for (std::size_t i = 0; i < contours[k].size(); ++i) {
          const auto& segment = contours[k][i];

          const auto& s = segment.source();
          const auto& t = segment.target();
          
          neighbor_query(s, neighbors);
          const std::size_t gr_idxs = pair_range[neighbors[0]].second;

          neighbor_query(t, neighbors);
          const std::size_t gr_idxt = pair_range[neighbors[0]].second;

          const auto& longests = longest_outer[gr_idxs];
          const auto& longestt = longest_outer[gr_idxt];

          const FT angles = angle_degree_2(longests, segment);
          const FT angles_2 = get_angle_2(angles);

          const FT anglet = angle_degree_2(longestt, segment);
          const FT anglet_2 = get_angle_2(anglet);

          const FT abs_angles_2 = CGAL::abs(angles_2);
          const FT abs_anglet_2 = CGAL::abs(anglet_2);

          if (
            abs_angles_2 <= FT(45) &&
            abs_anglet_2 <= FT(45) && 
            abs_angles_2 <= abs_anglet_2) {
        
            m_groups[k][i] = gr_idxs;
            continue;
          }

          if (
            abs_angles_2 <= FT(45) &&
            abs_anglet_2 <= FT(45) && 
            abs_angles_2 > abs_anglet_2) {
        
            m_groups[k][i] = gr_idxt;
            continue;
          }

          if (
            abs_angles_2 > FT(45) &&
            abs_anglet_2 > FT(45) && 
            abs_angles_2 > abs_anglet_2) {
        
            m_groups[k][i] = gr_idxs;
            continue;
          }

          if (
            abs_angles_2 > FT(45) &&
            abs_anglet_2 > FT(45) && 
            abs_angles_2 <= abs_anglet_2) {
        
            m_groups[k][i] = gr_idxt;
            continue;
          }

          if (
            abs_angles_2 < FT(45) &&
            abs_anglet_2 > FT(45) && 
            abs_angles_2 < ( FT(90) - abs_anglet_2 ) ) {
        
            m_groups[k][i] = gr_idxs;
            continue;
          }

          if (
            abs_angles_2 < FT(45) &&
            abs_anglet_2 > FT(45) && 
            abs_angles_2 >= ( FT(90) - abs_anglet_2 ) ) {
        
            m_groups[k][i] = gr_idxt;
            continue;
          }

          if (
            abs_angles_2 > FT(45) &&
            abs_anglet_2 < FT(45) && 
            abs_anglet_2 < ( FT(90) - abs_angles_2 ) ) {
        
            m_groups[k][i] = gr_idxt;
            continue;
          }

          if (
            abs_angles_2 > FT(45) &&
            abs_anglet_2 < FT(45) && 
            abs_anglet_2 >= ( FT(90) - abs_angles_2 ) ) {
        
            m_groups[k][i] = gr_idxs;
            continue;
          }

          m_groups[k][i] = 0;
        }
      }

      /*
      if (are_not_filled(m_groups)) {
        assign_groups(
          0, longest_outer, contours, FT(15), FT(75), m_groups);
        assign_groups(
          0, longest_outer, contours, FT(25), FT(75), m_groups);
        m_bounds = bounds_outer; m_longest = longest_outer;
      }

      if (are_not_filled(m_groups)) {
        assign_groups(
          m_longest.size(), longest_inner, contours, FT(15), FT(75), m_groups);

        for (const auto& l : longest_inner)
          m_longest.push_back(l);
        for (const auto& b : bounds_inner)
          m_bounds.push_back(b);
      }

      if (are_not_filled(m_groups)) {
        for (std::size_t k = 0; k < m_groups.size(); ++k) {
          for (std::size_t i = 0; i < m_groups[k].size(); ++i) {
            if (m_groups[k][i] == std::size_t(-1)) {
              m_groups[k][i] = 0; // the longest outer segment
            }
          }
        }
      } */
    }

    bool are_not_filled(
      const std::vector<Indices>& groups) {

      for (const auto& group : groups)
        for (const std::size_t value : group)
          if (value == std::size_t(-1))
            return true;
      return false;
    }

    void assign_groups(
      const std::size_t seed,
      const std::vector<Segment_2>& longest,
      const std::vector< std::vector<Segment_2> >& contours,
      const FT bound_min, const FT bound_max,
      std::vector<Indices>& groups) {

      set_closest_groups(
        seed, longest, contours, bound_min, bound_max, groups);
    }

    template<typename T>
    void make_default_groups(
      const std::vector< std::vector<T> >& contours,
      const std::size_t value,
      std::vector<Indices>& groups) {

      groups.clear(); groups.resize(contours.size());
      for (std::size_t k = 0; k < contours.size(); ++k)
        groups[k].resize(contours[k].size(), value);
    }

    void create_contours_from_segments(
      const std::vector<Segment_2>& segments,
      std::vector< std::vector<Seg_pair> >& contours) {

      contours.clear();
      std::vector<Seg_pair> contour;
      for (const auto& segment : segments) {
        const auto& s = segment.source();
        const auto& t = segment.target();
        
        if (internal::distance(s, t) > m_min_length * FT(2))
          contour.push_back(std::make_pair(segment, true));
        else 
          contour.push_back(std::make_pair(segment, false));
      }
      contours.push_back(contour);
    }

    void get_multiple_directions(
      const std::vector<Segment_2>& segments,
      const std::vector< std::vector<Seg_pair> >& contours,
      std::vector<FT_pair>& bounds,
      std::vector<Segment_2>& longest,
      std::vector<Indices>& groups) {

      make_default_groups(contours, std::size_t(-1), groups);

      std::vector<Size_pair> input;
      for (std::size_t k = 0; k < contours.size(); ++k)
        for (std::size_t i = 0; i < contours[k].size(); ++i)
          input.push_back(std::make_pair(k, i));
      
      sort_input(contours, input);
      std::vector<bool> states(input.size(), false);

      bool apply = true; std::size_t gr_idx = 0;
      do {
        apply = get_next_direction(
          contours, input, gr_idx, states, bounds, longest, groups);
          ++gr_idx;
      } while (apply);

      if (longest.size() == 0) {
        bounds.push_back(std::make_pair(FT(45), FT(45)));
        
        const std::size_t seg_idx = find_longest_segment(segments);
        longest.push_back(segments[seg_idx]);

        make_default_groups(contours, 0, groups);
      }
    }

    void sort_input(
      const std::vector< std::vector<Seg_pair> >& contours,
      std::vector<Size_pair>& input) {

      std::sort(input.begin(), input.end(), 
      [&contours](const Size_pair& a, const Size_pair& b) -> bool { 
        const FT length_1 = (contours[a.first][a.second]).first.squared_length();
        const FT length_2 = (contours[b.first][b.second]).first.squared_length();
        return length_1 > length_2;
      });
    }

    bool get_next_direction(
      const std::vector< std::vector<Seg_pair> >& contours,
      const std::vector<Size_pair>& input,
      const std::size_t gr_idx,
      std::vector<bool>& states,
      std::vector<FT_pair>& bounds,
      std::vector<Segment_2>& longest,
      std::vector<Indices>& groups) {

      std::size_t longest_idx = std::size_t(-1);
      for (std::size_t i = 0; i < states.size(); ++i) {
        if (!states[i] && contours[input[i].first][input[i].second].second) {
          longest_idx = i; break;
        }
      }
      if (longest_idx == std::size_t(-1))
        return false;
      
      const auto& longest_pair = input[longest_idx];
      const Segment_2& longest_segment = 
        contours[longest_pair.first][longest_pair.second].first;

      for (std::size_t i = 0; i < states.size(); ++i) {
        if (i == longest_idx) {
          groups[longest_pair.first][longest_pair.second] = gr_idx;
          states[i] = true; continue;
        }

        const auto& pair = input[i];
        if (!states[i] && contours[pair.first][pair.second].second) {
          const auto& segment = contours[pair.first][pair.second].first;

          const FT angle = angle_degree_2(longest_segment, segment);
          const FT angle_2 = get_angle_2(angle);

          if ( 
            (CGAL::abs(angle_2) <= m_bound_min) ||
            (CGAL::abs(angle_2) >= m_bound_max) )  {

            groups[pair.first][pair.second] = gr_idx;
            states[i] = true; continue;
          }
        }
      }

      longest.push_back(longest_segment);
      bounds.push_back(std::make_pair(FT(45), FT(45)));

      return true;
    }

    void compute_longest_direction(
      const std::vector<Segment_2>& outer_segments,
      const std::vector< std::vector<Segment_2> >& contours) {

      m_bounds.clear();
      m_bounds.resize(1);
      m_bounds[0] = std::make_pair(FT(45), FT(45));

      const std::size_t seg_idx = find_longest_segment(outer_segments);

      m_longest.clear();
      m_longest.resize(1);
      m_longest[0] = outer_segments[seg_idx];

      m_groups.clear();
      m_groups.resize(contours.size());
      for (std::size_t k = 0; k < contours.size(); ++k) {
        m_groups[k].resize(contours[k].size(), std::size_t(-1));
        for (std::size_t i = 0; i < contours[k].size(); ++i) {
          m_groups[k][i] = 0;
        }
      }
    }

    void regularize_contours(
      std::vector< std::vector<Segment_2> >& contours) {
      
      if (m_angle_bound == FT(0))
        return;

      for (std::size_t k = 0; k < contours.size(); ++k) {
        auto& contour = contours[k];
        rotate_contour(k, contour);
        if (contour.size() >= 3)
          correct_contour_n(k, contour);
      }
    }

    void merge_closest(std::vector<Segment_2>& segments) {
      
      std::vector<Segment_2> merged;

      std::vector<bool> states(segments.size(), false);
      std::vector<Segment_2> group; std::size_t num_groups = 0;
      for (std::size_t i = 0; i < segments.size(); ++i) {
        if (states[i]) continue;
        
        group.clear();
        const auto& segment = segments[i];
        states[i] = true;
        group.push_back(segment);
        const auto& p = segment.source();

        for (std::size_t j = 0; j < segments.size(); ++j) {
          if (states[j]) continue;
          
          const FT angle = angle_degree_2(segment, segments[j]);
          const FT angle_2 = get_angle_2(angle);

          if (CGAL::abs(angle_2) <= m_angle_threshold) {
            
            const Line_2 line = Line_2(
              segments[j].source(), segments[j].target());
              
            const auto q = line.projection(p);
            const FT dist = internal::distance(p, q);
            
            if (dist <= m_ordinate_bound) {
              group.push_back(segments[j]); states[j] = true;
            }
          }
        }

        auto central = find_central_segment(group);
        const Line_2 line = Line_2(central.source(), central.target());
        
        FT sum_length = FT(0);
        std::vector<Point_2> points;
        for (const auto& seg : group) {
          const Point_2 p = line.projection(seg.source());
          const Point_2 q = line.projection(seg.target());
          points.push_back(p);
          points.push_back(q);
          sum_length += internal::distance(p, q);
        }
        update_segment(points, central);
        merged.push_back(central);

        ++num_groups;
      }
      segments = merged;
      std::cout << "Num collinear groups: " << num_groups << std::endl;
    }

  private:
    const FT m_min_length;
    const FT m_angle_bound;
    const FT m_ordinate_bound;

    const FT m_pi;
    const FT m_angle_threshold;
    const FT m_bound_min, m_bound_max;

    std::vector<FT_pair> m_bounds;
    std::vector<Segment_2> m_longest;
    std::vector<Indices> m_groups;

    std::size_t find_longest_segment(
      const std::vector<Segment_2>& segments) {

      std::size_t seg_idx = std::size_t(-1);
      FT max_length = -FT(1);
      for (std::size_t i = 0; i < segments.size(); ++i) {
          
        const FT length = segments[i].squared_length();
        if (length > max_length) {

          max_length = length;
          seg_idx = i;
        }
      }
      return seg_idx;
    }

    void rotate_contour(
      const std::size_t k,
      std::vector<Segment_2>& contour) {

      for (std::size_t i = 0; i < contour.size(); ++i) {
        const std::size_t gr_idx = m_groups[k][i];
        if (gr_idx == std::size_t(-1))
          continue;

        auto& si = contour[i];
        const auto& longest = m_longest[gr_idx];
        const auto& bounds = m_bounds[gr_idx];
        rotate_segment(longest, bounds, si);
      }
    }

    void rotate_segment(
      const Segment_2& longest,
      const FT_pair& bounds,
      Segment_2& si) {

      const FT angle = angle_degree_2(longest, si);
      const FT angle_2 = get_angle_2(angle);

      if (CGAL::abs(angle_2) <= bounds.first) {
        rotate(angle, FT(180), longest, si); // parallel case
        return;
      }
      if (CGAL::abs(angle_2) >= bounds.second) {
        rotate(angle, FT(90), longest, si); // orthogonal case
        return;
      }
    }

    FT angle_degree_2(
      const Segment_2& longest, const Segment_2& si) {

      const Vector_2 v1 = si.to_vector();
      const Vector_2 v2 = -longest.to_vector();

		  const FT det = CGAL::determinant(v1, v2);
		  const FT dot = CGAL::scalar_product(v1, v2);
      const FT angle_rad = static_cast<FT>(
        std::atan2(CGAL::to_double(det), CGAL::to_double(dot)));
      const FT angle_deg = angle_rad * FT(180) / m_pi; 
      return angle_deg;
    }

    FT get_angle_2(const FT angle) {
      
      FT angle_2 = angle;
      if (angle_2 > FT(90)) angle_2 = FT(180) - angle_2;
      else if (angle_2 < -FT(90)) angle_2 = FT(180) + angle_2;
      return angle_2;
    }

    void rotate(
      const FT angle_2, 
      const FT ref_angle_2,
      const Segment_2& longest, 
      Segment_2& si) {

      FT angle = angle_2;
      if (angle < FT(0)) angle = angle + ref_angle_2;
      else if (angle > FT(0)) angle = angle - ref_angle_2;

      Point_2 source_i = si.source();
      Point_2 target_i = si.target();
      const Point_2 b = internal::middle_point_2(source_i, target_i);
      const FT angle_rad = angle * m_pi / FT(180); 
      internal::rotate_point_2(angle_rad, b, source_i);
      internal::rotate_point_2(angle_rad, b, target_i);
      si = Segment_2(source_i, target_i);
    }

    void correct_contour_2(
      const std::size_t k,
      std::vector<Segment_2>& contour) {

      const std::size_t seg_idx = find_longest_segment(contour);
      
      const std::size_t i = (seg_idx == 0 ? 1 : 0);
      const std::size_t j =  seg_idx;
      
      const std::size_t gr_idx = m_groups[k][i];
      if (gr_idx == std::size_t(-1))
        return;

      auto& si = contour[i];
      const auto& sj = contour[j];

      FT angle_ij = FT(0);
      if (i > j) angle_ij = angle_degree_2(sj, si);
      else angle_ij = angle_degree_2(si, sj);

      const FT angle_ij_2 = get_angle_2(angle_ij);
      if (CGAL::abs(angle_ij_2) <= m_angle_threshold) {
        
        FT angle = FT(0);
        if (i > j) angle = angle_degree_2(sj, si);
        else angle = angle_degree_2(si, sj);
        
        rotate(angle, FT(90), sj, si); // orthogonal case
        return;
      }
    }

    void correct_contour_n(
      const std::size_t k,
      std::vector<Segment_2>& contour) {

      const std::size_t n = contour.size();
      for (std::size_t i = 1; i < n - 1; ++i) {
        const std::size_t gr_idx = m_groups[k][i];
        if (gr_idx == std::size_t(-1))
          continue;

        const std::size_t im = i - 1;
        const std::size_t ip = i + 1;

        auto& si = contour[i];
        const auto& sm = contour[im];
        const auto& sp = contour[ip];

        const FT length = internal::distance(si.source(), si.target());
        if (length <= m_min_length)
          correct_segment(sm, si, sp);
      }
    }

    void correct_segment(
      const Segment_2& sm, Segment_2& si, const Segment_2& sp) {

      const FT angle_mp = angle_degree_2(sm, sp);
      const FT angle_mp_2 = get_angle_2(angle_mp);

      if (CGAL::abs(angle_mp_2) <= m_angle_threshold) {
        const FT angle = angle_degree_2(sm, si);
        rotate(angle, FT(90), sm, si); // orthogonal case
        return;
      }
    }

    Segment_2 find_central_segment(
      const std::vector<Segment_2>& segments) {

      Point_2 source, target;
      FT x1 = FT(0), y1 = FT(0);
      FT x2 = FT(0), y2 = FT(0);
      for (const auto& segment : segments) {
        x1 += segment.source().x();
        x2 += segment.target().x();

        y1 += segment.source().y();
        y2 += segment.target().y();
      }

      const FT size = static_cast<FT>(segments.size());
      x1 /= size; y1 /= size;
      x2 /= size; y2 /= size;

      return Segment_2(Point_2(x1, y1), Point_2(x2, y2));
    }

    void update_segment(
      const std::vector<Point_2>& points,
      Segment_2& segment) {

      FT min_proj_value =  internal::max_value<FT>();
      FT max_proj_value = -internal::max_value<FT>();

      const Vector_2 ref_vector = segment.to_vector();
      Point_2 ref_point;
      internal::compute_barycenter_2(points, ref_point);
      
      Point_2 p, q;
      for (const auto& point : points) {
        const Vector_2 curr_vector(ref_point, point);
        const FT value = CGAL::scalar_product(curr_vector, ref_vector);
        
        if (value < min_proj_value) {
          min_proj_value = value;
          p = point; }
        if (value > max_proj_value) {
          max_proj_value = value;
          q = point; }
      }
      segment = Segment_2(p, q);
    }

    void set_closest_groups(
      const std::size_t seed,
      const std::vector<Segment_2>& longest,
      const std::vector< std::vector<Segment_2> >& contours,
      const FT bound_min, const FT bound_max,
      std::vector<Indices>& groups) {

      for (std::size_t k = 0; k < contours.size(); ++k) {
        for (std::size_t i = 0; i < contours[k].size(); ++i) {
          if (groups[k][i] != std::size_t(-1)) continue;

          FT angle_min = internal::max_value<FT>();
          std::size_t idx_min = std::size_t(-1);

          const auto& segment = contours[k][i];
          for (std::size_t j = 0; j < longest.size(); ++j) {
  
            const FT angle = angle_degree_2(longest[j], segment);
            const FT angle_2 = get_angle_2(angle);

            const FT abs_angle_2 = CGAL::abs(angle_2);
            if (
              abs_angle_2 <= bound_min && 
              abs_angle_2 < angle_min ) {
              
              angle_min = abs_angle_2;
              idx_min = seed + j;
            }

            if (
              abs_angle_2 >= bound_max && 
              angle_min > FT(90) - abs_angle_2 ) {
              
              angle_min = FT(90) - abs_angle_2;
              idx_min = seed + j;
            }
          }

          const auto& s = segment.source();
          const auto& t = segment.target();
        
          if (internal::distance(s, t) > m_min_length * FT(2))
            groups[k][i] = idx_min;
        }
      }
    }

    void unify_along_contours(
      const std::vector< std::vector<Seg_pair> >& contours,
      std::vector<Indices>& groups) {

      for (std::size_t k = 0; k < contours.size(); ++k) {
        for (std::size_t i = 0; i < contours[k].size(); ++i) {
          if (contours[k][i].second)
            continue;
          
          if (groups[k][i] == std::size_t(-1)) {

            const std::size_t m = contours[k].size();
            std::size_t im = (i + m - 1) % m;
            std::size_t ip = (i + 1) % m;

            bool stop = false;
            std::size_t max_count = 0;
            do {

              if (contours[k][im].second) {
                groups[k][i] = groups[k][im]; break;
              }

              if (contours[k][ip].second) {
                groups[k][i] = groups[k][ip]; break;
              }

              im = (im + m - 1) % m;
              ip = (ip + 1) % m;

              if (im == i || ip == i) stop = true;
              ++max_count;

            } while (!stop && max_count < m * 2);
            if (stop || max_count >= m * 2)
              groups[k][i] = 0;
          }
        }
      }
    }
  };

} // internal
} // Levels_of_detail
} // CGAL

#endif // CGAL_LEVELS_OF_DETAIL_INTERNAL_SEGMENT_REGULARIZER_H
