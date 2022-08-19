// Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2.0,
// as published by the Free Software Foundation.
//
// This program is also distributed with certain software (including
// but not limited to OpenSSL) that is licensed under separate terms,
// as designated in a particular file or component or in included license
// documentation.  The authors of MySQL hereby grant you an additional
// permission to link the program and your derivative works with the
// separately licensed software that they have included with MySQL.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.

/// @file
///
/// This file implements the touches functor and function.

#include <cstddef>  // std::size_t
#include <memory>   // std::unique_ptr

#include <boost/geometry.hpp>

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/box.h"
#include "sql/gis/box_traits.h"
#include "sql/gis/gc_utils.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_traits.h"
#include "sql/gis/mbr_utils.h"
#include "sql/gis/relops.h"
#include "sql/gis/touches_functor.h"
#include "sql/gis/within_functor.h"
#include "sql/sql_exception_handler.h"  // handle_gis_exception

namespace bg = boost::geometry;

namespace gis {

/// Apply a Touches functor to two geometries, which both may be geometry
/// collections, and return the booelan result of the functor applied on each
/// combination of elements in the collections.
///
/// @tparam GC Coordinate specific gometry collection type.
///
/// @param f Functor to apply.
/// @param g1 First geometry.
/// @param g2 Second geometry.
///
/// @retval true g1 touches g2.
/// @retval false g1 doesn't touch g2.
template <typename GC>
static bool geometry_collection_apply_touches(const Touches &f,
                                              const Geometry *g1,
                                              const Geometry *g2) {
  boost::geometry::strategy::within::geographic_winding<Geographic_point>
  geographic_pl_pa_strategy(
      bg::srs::spheroid<double>(f.semi_major(), f.semi_minor()));
  boost::geometry::strategy::intersection::geographic_segments<>
  geographic_ll_la_aa_strategy(
      bg::srs::spheroid<double>(f.semi_major(), f.semi_minor()));

  if (g1->type() == Geometry_type::kGeometrycollection) {
    if (g2->type() == Geometry_type::kGeometrycollection) {
      std::unique_ptr<Multipoint> g1_mpt;
      std::unique_ptr<Multilinestring> g1_mls;
      std::unique_ptr<Multipolygon> g1_mpy;
      std::unique_ptr<Multipoint> g2_mpt;
      std::unique_ptr<Multilinestring> g2_mls;
      std::unique_ptr<Multipolygon> g2_mpy;
      split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
               &g1_mpy);
      gc_union(f.semi_major(), f.semi_minor(), &g1_mpt, &g1_mls, &g1_mpy);
      split_gc(down_cast<const Geometrycollection *>(g2), &g2_mpt, &g2_mls,
               &g2_mpy);
      gc_union(f.semi_major(), f.semi_minor(), &g2_mpt, &g2_mls, &g2_mpy);

      if (!g1_mpt->empty() && g1_mls->empty() && g1_mpy->empty() &&
          !g2_mpt->empty() && g2_mls->empty() && g2_mpy->empty())
        throw null_value_exception();

      // Check that at least one part of g1 touches at least one part of g2.
      if (!((!g1_mpt->empty() && !g2_mls->empty() &&
             f(g1_mpt.get(), g2_mls.get())) ||
            (!g1_mpt->empty() && !g2_mpy->empty() &&
             f(g1_mpt.get(), g2_mpy.get())) ||
            (!g1_mls->empty() && !g2_mpt->empty() &&
             f(g1_mls.get(), g2_mpt.get())) ||
            (!g1_mls->empty() && !g2_mls->empty() &&
             f(g1_mls.get(), g2_mls.get())) ||
            (!g1_mls->empty() && !g2_mpy->empty() &&
             f(g1_mls.get(), g2_mpy.get())) ||
            (!g1_mpy->empty() && !g2_mpt->empty() &&
             f(g1_mpy.get(), g2_mpt.get())) ||
            (!g1_mpy->empty() && !g2_mls->empty() &&
             f(g1_mpy.get(), g2_mls.get())) ||
            (!g1_mpy->empty() && !g2_mpy->empty() &&
             f(g1_mpy.get(), g2_mpy.get()))))
        return false;

      // Check that the interiors of g1 and g2 are disjoint.
      boost::geometry::de9im::mask mask("T********");
      if (g1->coordinate_system() == Coordinate_system::kCartesian) {
        for (std::size_t i = 0;
             i < down_cast<Cartesian_multipoint *>(g1_mpt.get())->size(); i++) {
          auto &pt = (*down_cast<Cartesian_multipoint *>(g1_mpt.get()))[i];
          if (bg::relate(pt, *down_cast<Cartesian_multipoint *>(g2_mpt.get()),
                         mask) ||
              bg::relate(pt,
                         *down_cast<Cartesian_multilinestring *>(g2_mls.get()),
                         mask) ||
              bg::relate(pt, *down_cast<Cartesian_multipolygon *>(g2_mpy.get()),
                         mask))
            return false;
        }
        for (std::size_t i = 0;
             i < down_cast<Cartesian_multipoint *>(g2_mpt.get())->size(); i++) {
          auto &pt = (*down_cast<Cartesian_multipoint *>(g2_mpt.get()))[i];
          if (bg::relate(pt,
                         *down_cast<Cartesian_multilinestring *>(g1_mls.get()),
                         mask) ||
              bg::relate(pt, *down_cast<Cartesian_multipolygon *>(g1_mpy.get()),
                         mask))
            return false;
        }

        if (bg::relate(*down_cast<Cartesian_multilinestring *>(g1_mls.get()),
                       *down_cast<Cartesian_multilinestring *>(g2_mls.get()),
                       mask) ||
            bg::relate(*down_cast<Cartesian_multilinestring *>(g1_mls.get()),
                       *down_cast<Cartesian_multipolygon *>(g2_mpy.get()),
                       mask) ||
            bg::relate(*down_cast<Cartesian_multipolygon *>(g1_mpy.get()),
                       *down_cast<Cartesian_multilinestring *>(g2_mls.get()),
                       mask) ||
            bg::relate(*down_cast<Cartesian_multipolygon *>(g1_mpy.get()),
                       *down_cast<Cartesian_multipolygon *>(g2_mpy.get()),
                       mask))
          return false;
      } else {
        DBUG_ASSERT(g1->coordinate_system() == Coordinate_system::kGeographic);
        for (std::size_t i = 0;
             i < down_cast<Geographic_multipoint *>(g1_mpt.get())->size();
             i++) {
          auto &pt = (*down_cast<Geographic_multipoint *>(g1_mpt.get()))[i];
          if (bg::relate(pt, *down_cast<Geographic_multipoint *>(g2_mpt.get()),
                         mask) ||
              bg::relate(pt,
                         *down_cast<Geographic_multilinestring *>(g2_mls.get()),
                         mask, geographic_pl_pa_strategy) ||
              bg::relate(pt,
                         *down_cast<Geographic_multipolygon *>(g2_mpy.get()),
                         mask, geographic_pl_pa_strategy))
            return false;
        }
        for (std::size_t i = 0;
             i < down_cast<Geographic_multipoint *>(g2_mpt.get())->size();
             i++) {
          auto &pt = (*down_cast<Geographic_multipoint *>(g2_mpt.get()))[i];
          if (bg::relate(pt,
                         *down_cast<Geographic_multilinestring *>(g1_mls.get()),
                         mask, geographic_pl_pa_strategy) ||
              bg::relate(pt,
                         *down_cast<Geographic_multipolygon *>(g1_mpy.get()),
                         mask, geographic_pl_pa_strategy))
            return false;
        }

        if (bg::relate(*down_cast<Geographic_multilinestring *>(g1_mls.get()),
                       *down_cast<Geographic_multilinestring *>(g2_mls.get()),
                       mask, geographic_ll_la_aa_strategy) ||
            bg::relate(*down_cast<Geographic_multilinestring *>(g1_mls.get()),
                       *down_cast<Geographic_multipolygon *>(g2_mpy.get()),
                       mask, geographic_ll_la_aa_strategy) ||
            bg::relate(*down_cast<Geographic_multipolygon *>(g1_mpy.get()),
                       *down_cast<Geographic_multilinestring *>(g2_mls.get()),
                       mask, geographic_ll_la_aa_strategy) ||
            bg::relate(*down_cast<Geographic_multipolygon *>(g1_mpy.get()),
                       *down_cast<Geographic_multipolygon *>(g2_mpy.get()),
                       mask, geographic_ll_la_aa_strategy))
          return false;
      }

      return true;
    } else {
      return f(g2, g1);
    }
  } else {
    if (g2->type() == Geometry_type::kGeometrycollection) {
      std::unique_ptr<Multipoint> g2_mpt;
      std::unique_ptr<Multilinestring> g2_mls;
      std::unique_ptr<Multipolygon> g2_mpy;
      split_gc(down_cast<const Geometrycollection *>(g2), &g2_mpt, &g2_mls,
               &g2_mpy);
      gc_union(f.semi_major(), f.semi_minor(), &g2_mpt, &g2_mls, &g2_mpy);

      // Check that at g1 touches at least one part of g2.
      if (!((!g2_mpt->empty() && f(g1, g2_mpt.get())) ||
            (!g2_mls->empty() && f(g1, g2_mls.get())) ||
            (!g2_mpy->empty() && f(g1, g2_mpy.get()))))
        return false;

      // Check that the interiors of g1 and g2 are disjoint.
      boost::geometry::de9im::mask mask("T********");
      if (g1->coordinate_system() == Coordinate_system::kCartesian) {
        switch (g1->type()) {
          case Geometry_type::kPoint:
            if (!g2_mpt->empty() && g2_mls->empty() && g2_mpy->empty())
              throw null_value_exception();
            if (bg::relate(*down_cast<const Cartesian_point *>(g1),
                           *down_cast<Cartesian_multipoint *>(g2_mpt.get()),
                           mask) ||
                bg::relate(
                    *down_cast<const Cartesian_point *>(g1),
                    *down_cast<Cartesian_multilinestring *>(g2_mls.get()),
                    mask) ||
                bg::relate(*down_cast<const Cartesian_point *>(g1),
                           *down_cast<Cartesian_multipolygon *>(g2_mpy.get()),
                           mask))
              return false;
            break;
          case Geometry_type::kLinestring:
            for (std::size_t i = 0;
                 i < down_cast<Cartesian_multipoint *>(g2_mpt.get())->size();
                 i++) {
              auto &pt = (*down_cast<Cartesian_multipoint *>(g2_mpt.get()))[i];
              if (bg::relate(pt, *down_cast<const Cartesian_linestring *>(g1),
                             mask))
                return false;
            }
            if (bg::relate(
                    *down_cast<const Cartesian_linestring *>(g1),
                    *down_cast<Cartesian_multilinestring *>(g2_mls.get()),
                    mask) ||
                bg::relate(*down_cast<const Cartesian_linestring *>(g1),
                           *down_cast<Cartesian_multipolygon *>(g2_mpy.get()),
                           mask))
              return false;
            break;
          case Geometry_type::kPolygon:
            for (std::size_t i = 0;
                 i < down_cast<Cartesian_multipoint *>(g2_mpt.get())->size();
                 i++) {
              auto &pt = (*down_cast<Cartesian_multipoint *>(g2_mpt.get()))[i];
              if (bg::relate(pt, *down_cast<const Cartesian_polygon *>(g1),
                             mask))
                return false;
            }
            if (bg::relate(
                    *down_cast<const Cartesian_polygon *>(g1),
                    *down_cast<Cartesian_multilinestring *>(g2_mls.get()),
                    mask) ||
                bg::relate(*down_cast<const Cartesian_polygon *>(g1),
                           *down_cast<Cartesian_multipolygon *>(g2_mpy.get()),
                           mask))
              return false;
            break;
          case Geometry_type::kMultipoint:
            if (!g2_mpt->empty() && g2_mls->empty() && g2_mpy->empty())
              throw null_value_exception();
            for (std::size_t i = 0;
                 i < down_cast<const Cartesian_multipoint *>(g1)->size(); i++) {
              auto &pt = (*down_cast<const Cartesian_multipoint *>(g1))[i];
              if (bg::relate(
                      pt, *down_cast<Cartesian_multilinestring *>(g2_mls.get()),
                      mask) ||
                  bg::relate(pt,
                             *down_cast<Cartesian_multipolygon *>(g2_mpy.get()),
                             mask))
                return false;
            }
            if (bg::relate(*down_cast<const Cartesian_multipoint *>(g1),
                           *down_cast<Cartesian_multipoint *>(g2_mpt.get()),
                           mask))
              return false;
            break;
          case Geometry_type::kMultilinestring:
            for (std::size_t i = 0;
                 i < down_cast<Cartesian_multipoint *>(g2_mpt.get())->size();
                 i++) {
              auto &pt = (*down_cast<Cartesian_multipoint *>(g2_mpt.get()))[i];
              if (bg::relate(pt,
                             *down_cast<const Cartesian_multilinestring *>(g1),
                             mask))
                return false;
            }
            if (bg::relate(
                    *down_cast<const Cartesian_multilinestring *>(g1),
                    *down_cast<Cartesian_multilinestring *>(g2_mls.get()),
                    mask) ||
                bg::relate(*down_cast<const Cartesian_multilinestring *>(g1),
                           *down_cast<Cartesian_multipolygon *>(g2_mpy.get()),
                           mask))
              return false;
            break;
          case Geometry_type::kMultipolygon:
            for (std::size_t i = 0;
                 i < down_cast<Cartesian_multipoint *>(g2_mpt.get())->size();
                 i++) {
              auto &pt = (*down_cast<Cartesian_multipoint *>(g2_mpt.get()))[i];
              if (bg::relate(pt, *down_cast<const Cartesian_multipolygon *>(g1),
                             mask))
                return false;
            }
            if (bg::relate(
                    *down_cast<const Cartesian_multipolygon *>(g1),
                    *down_cast<Cartesian_multilinestring *>(g2_mls.get()),
                    mask) ||
                bg::relate(*down_cast<const Cartesian_multipolygon *>(g1),
                           *down_cast<Cartesian_multipolygon *>(g2_mpy.get()),
                           mask))
              return false;
            break;
          default:
            DBUG_ASSERT(false); /* purecov: inspected */
            return false;
        }
      } else {
        DBUG_ASSERT(g1->coordinate_system() == Coordinate_system::kGeographic);
        switch (g1->type()) {
          case Geometry_type::kPoint:
            if (bg::relate(*down_cast<const Geographic_point *>(g1),
                           *down_cast<Geographic_multipoint *>(g2_mpt.get()),
                           mask) ||
                bg::relate(
                    *down_cast<const Geographic_point *>(g1),
                    *down_cast<Geographic_multilinestring *>(g2_mls.get()),
                    mask, geographic_pl_pa_strategy) ||
                bg::relate(*down_cast<const Geographic_point *>(g1),
                           *down_cast<Geographic_multipolygon *>(g2_mpy.get()),
                           mask, geographic_pl_pa_strategy))
              return false;
            break;
          case Geometry_type::kLinestring:
            for (std::size_t i = 0;
                 i < down_cast<Geographic_multipoint *>(g2_mpt.get())->size();
                 i++) {
              auto &pt = (*down_cast<Geographic_multipoint *>(g2_mpt.get()))[i];
              if (bg::relate(pt, *down_cast<const Geographic_linestring *>(g1),
                             mask, geographic_pl_pa_strategy))
                return false;
            }
            if (bg::relate(
                    *down_cast<const Geographic_linestring *>(g1),
                    *down_cast<Geographic_multilinestring *>(g2_mls.get()),
                    mask, geographic_ll_la_aa_strategy) ||
                bg::relate(*down_cast<const Geographic_linestring *>(g1),
                           *down_cast<Geographic_multipolygon *>(g2_mpy.get()),
                           mask, geographic_ll_la_aa_strategy))
              return false;
            break;
          case Geometry_type::kPolygon:
            for (std::size_t i = 0;
                 i < down_cast<Geographic_multipoint *>(g2_mpt.get())->size();
                 i++) {
              auto &pt = (*down_cast<Geographic_multipoint *>(g2_mpt.get()))[i];
              if (bg::relate(pt, *down_cast<const Geographic_polygon *>(g1),
                             mask, geographic_pl_pa_strategy))
                return false;
            }
            if (bg::relate(
                    *down_cast<const Geographic_polygon *>(g1),
                    *down_cast<Geographic_multilinestring *>(g2_mls.get()),
                    mask, geographic_ll_la_aa_strategy) ||
                bg::relate(*down_cast<const Geographic_polygon *>(g1),
                           *down_cast<Geographic_multipolygon *>(g2_mpy.get()),
                           mask, geographic_ll_la_aa_strategy))
              return false;
            break;
          case Geometry_type::kMultipoint:
            for (std::size_t i = 0;
                 i < down_cast<const Geographic_multipoint *>(g1)->size();
                 i++) {
              auto &pt = (*down_cast<const Geographic_multipoint *>(g1))[i];
              if (bg::relate(
                      pt,
                      *down_cast<Geographic_multilinestring *>(g2_mls.get()),
                      mask, geographic_pl_pa_strategy) ||
                  bg::relate(
                      pt, *down_cast<Geographic_multipolygon *>(g2_mpy.get()),
                      mask, geographic_pl_pa_strategy))
                return false;
            }
            // Default strategy is OK for multipoint-multipoint.
            if (bg::relate(*down_cast<const Geographic_multipoint *>(g1),
                           *down_cast<Geographic_multipoint *>(g2_mpt.get()),
                           mask))
              return false;
            break;
          case Geometry_type::kMultilinestring:
            for (std::size_t i = 0;
                 i < down_cast<Geographic_multipoint *>(g2_mpt.get())->size();
                 i++) {
              auto &pt = (*down_cast<Geographic_multipoint *>(g2_mpt.get()))[i];
              if (bg::relate(pt,
                             *down_cast<const Geographic_multilinestring *>(g1),
                             mask, geographic_pl_pa_strategy))
                return false;
            }
            if (bg::relate(
                    *down_cast<const Geographic_multilinestring *>(g1),
                    *down_cast<Geographic_multilinestring *>(g2_mls.get()),
                    mask, geographic_ll_la_aa_strategy) ||
                bg::relate(*down_cast<const Geographic_multilinestring *>(g1),
                           *down_cast<Geographic_multipolygon *>(g2_mpy.get()),
                           mask, geographic_ll_la_aa_strategy))
              return false;
            break;
          case Geometry_type::kMultipolygon:
            for (std::size_t i = 0;
                 i < down_cast<Geographic_multipoint *>(g2_mpt.get())->size();
                 i++) {
              auto &pt = (*down_cast<Geographic_multipoint *>(g2_mpt.get()))[i];
              if (bg::relate(pt,
                             *down_cast<const Geographic_multipolygon *>(g1),
                             mask, geographic_pl_pa_strategy))
                return false;
            }
            if (bg::relate(
                    *down_cast<const Geographic_multipolygon *>(g1),
                    *down_cast<Geographic_multilinestring *>(g2_mls.get()),
                    mask, geographic_ll_la_aa_strategy) ||
                bg::relate(*down_cast<const Geographic_multipolygon *>(g1),
                           *down_cast<Geographic_multipolygon *>(g2_mpy.get()),
                           mask, geographic_ll_la_aa_strategy))
              return false;
            break;
          default:
            DBUG_ASSERT(false); /* purecov: inspected */
            return false;
        }
      }

      return true;
    } else {
      return f(g1, g2);
    }
  }
}

Touches::Touches(double semi_major, double semi_minor)
    : m_semi_major(semi_major),
      m_semi_minor(semi_minor),
      m_geographic_pl_pa_strategy(
          bg::srs::spheroid<double>(semi_major, semi_minor)),
      m_geographic_ll_la_aa_strategy(
          bg::srs::spheroid<double>(semi_major, semi_minor)) {}

bool Touches::operator()(const Geometry *g1, const Geometry *g2) const {
  return apply(*this, g1, g2);
}

bool Touches::operator()(const Box *b1, const Box *b2) const {
  DBUG_ASSERT(b1->coordinate_system() == b2->coordinate_system());
  switch (b1->coordinate_system()) {
    case Coordinate_system::kCartesian:
      return eval(down_cast<const Cartesian_box *>(b1),
                  down_cast<const Cartesian_box *>(b2));
    case Coordinate_system::kGeographic:
      return eval(down_cast<const Geographic_box *>(b1),
                  down_cast<const Geographic_box *>(b2));
  }

  DBUG_ASSERT(false);
  return false;
}

bool Touches::eval(const Geometry *g1, const Geometry *g2) const {
  // All parameter type combinations have been implemented.
  DBUG_ASSERT(false);
  throw not_implemented_exception::for_non_projected(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// touches(Cartesian_point, *)

bool Touches::eval(const Cartesian_point *, const Cartesian_point *) const {
  // If dim(g1) == 0 and dim(g2) == 0, return NULL (SQL/MM 2015 Part 3,
  // Sect. 5.1.50).
  throw null_value_exception();
}

bool Touches::eval(const Cartesian_point *g1,
                   const Cartesian_linestring *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_point *g1,
                   const Cartesian_polygon *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_point *g1,
                   const Cartesian_geometrycollection *g2) const {
  return geometry_collection_apply_touches<Cartesian_geometrycollection>(
      *this, g1, g2);
}

bool Touches::eval(const Cartesian_point *,
                   const Cartesian_multipoint *) const {
  // If dim(g1) == 0 and dim(g2) == 0, return NULL (SQL/MM 2015 Part 3,
  // Sect. 5.1.50).
  throw null_value_exception();
}

bool Touches::eval(const Cartesian_point *g1,
                   const Cartesian_multilinestring *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_point *g1,
                   const Cartesian_multipolygon *g2) const {
  return bg::touches(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// touches(Cartesian_linestring, *)

bool Touches::eval(const Cartesian_linestring *g1,
                   const Cartesian_point *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_linestring *g1,
                   const Cartesian_linestring *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_linestring *g1,
                   const Cartesian_polygon *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_linestring *g1,
                   const Cartesian_geometrycollection *g2) const {
  return geometry_collection_apply_touches<Cartesian_geometrycollection>(
      *this, g1, g2);
}

bool Touches::eval(const Cartesian_linestring *g1,
                   const Cartesian_multipoint *g2) const {
  return eval(g2, g1);
}

bool Touches::eval(const Cartesian_linestring *g1,
                   const Cartesian_multilinestring *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_linestring *g1,
                   const Cartesian_multipolygon *g2) const {
  return bg::touches(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// touches(Cartesian_polygon, *)

bool Touches::eval(const Cartesian_polygon *g1,
                   const Cartesian_point *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_polygon *g1,
                   const Cartesian_linestring *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_polygon *g1,
                   const Cartesian_polygon *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_polygon *g1,
                   const Cartesian_geometrycollection *g2) const {
  return geometry_collection_apply_touches<Cartesian_geometrycollection>(
      *this, g1, g2);
}

bool Touches::eval(const Cartesian_polygon *g1,
                   const Cartesian_multipoint *g2) const {
  return eval(g2, g1);
}

bool Touches::eval(const Cartesian_polygon *g1,
                   const Cartesian_multilinestring *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_polygon *g1,
                   const Cartesian_multipolygon *g2) const {
  return bg::touches(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// touches(Cartesian_geometrycollection, *)

bool Touches::eval(const Cartesian_geometrycollection *g1,
                   const Geometry *g2) const {
  return geometry_collection_apply_touches<Cartesian_geometrycollection>(
      *this, g1, g2);
}

//////////////////////////////////////////////////////////////////////////////

// touches(Cartesian_multipoint, *)

bool Touches::eval(const Cartesian_multipoint *,
                   const Cartesian_point *) const {
  // If dim(g1) == 0 and dim(g2) == 0, return NULL (SQL/MM 2015 Part 3,
  // Sect. 5.1.50).
  throw null_value_exception();
}

bool Touches::eval(const Cartesian_multipoint *g1,
                   const Cartesian_linestring *g2) const {
  Within within(m_semi_major, m_semi_minor);
  bool touches = false;

  // At least one point in g1 has to touch g2, and none of the points in g1
  // may be within g2.
  for (auto &pt : *g1) {
    bool pt_touches = false;
    if (!touches) {
      pt_touches = bg::touches(pt, *g2);
      touches = pt_touches;
    }
    if (!pt_touches) {
      if (within(&pt, g2)) return false;
    }
  }

  return touches;
}

bool Touches::eval(const Cartesian_multipoint *g1,
                   const Cartesian_polygon *g2) const {
  Within within(m_semi_major, m_semi_minor);
  bool touches = false;

  // At least one point in g1 has to touch g2, and none of the points in g1
  // may be within g2.
  for (auto &pt : *g1) {
    bool pt_touches = false;
    if (!touches) {
      pt_touches = bg::touches(pt, *g2);
      touches = pt_touches;
    }
    if (!pt_touches) {
      if (within(&pt, g2)) return false;
    }
  }

  return touches;
}

bool Touches::eval(const Cartesian_multipoint *g1,
                   const Cartesian_geometrycollection *g2) const {
  return geometry_collection_apply_touches<Cartesian_geometrycollection>(
      *this, g1, g2);
}

bool Touches::eval(const Cartesian_multipoint *,
                   const Cartesian_multipoint *) const {
  // If dim(g1) == 0 and dim(g2) == 0, return NULL (SQL/MM 2015 Part 3,
  // Sect. 5.1.50).
  throw null_value_exception();
}

bool Touches::eval(const Cartesian_multipoint *g1,
                   const Cartesian_multilinestring *g2) const {
  Within within(m_semi_major, m_semi_minor);
  bool touches = false;

  // At least one point in g1 has to touch g2, and none of the points in g1
  // may be within g2.
  for (auto &pt : *g1) {
    bool pt_touches = false;
    if (!touches) {
      pt_touches = bg::touches(pt, *g2);
      touches = pt_touches;
    }
    if (!pt_touches) {
      if (within(&pt, g2)) return false;
    }
  }

  return touches;
}

bool Touches::eval(const Cartesian_multipoint *g1,
                   const Cartesian_multipolygon *g2) const {
  Within within(m_semi_major, m_semi_minor);
  bool touches = false;

  // At least one point in g1 has to touch g2, and none of the points in g1
  // may be within g2.
  for (auto &pt : *g1) {
    bool pt_touches = false;
    if (!touches) {
      pt_touches = bg::touches(pt, *g2);
      touches = pt_touches;
    }
    if (!pt_touches) {
      if (within(&pt, g2)) return false;
    }
  }

  return touches;
}

//////////////////////////////////////////////////////////////////////////////

// touches(Cartesian_multilinestring, *)

bool Touches::eval(const Cartesian_multilinestring *g1,
                   const Cartesian_point *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_multilinestring *g1,
                   const Cartesian_linestring *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_multilinestring *g1,
                   const Cartesian_polygon *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_multilinestring *g1,
                   const Cartesian_geometrycollection *g2) const {
  return geometry_collection_apply_touches<Cartesian_geometrycollection>(
      *this, g1, g2);
}

bool Touches::eval(const Cartesian_multilinestring *g1,
                   const Cartesian_multipoint *g2) const {
  return eval(g2, g1);
}

bool Touches::eval(const Cartesian_multilinestring *g1,
                   const Cartesian_multilinestring *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_multilinestring *g1,
                   const Cartesian_multipolygon *g2) const {
  return bg::touches(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// touches(Cartesian_multipolygon, *)

bool Touches::eval(const Cartesian_multipolygon *g1,
                   const Cartesian_point *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_multipolygon *g1,
                   const Cartesian_linestring *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_multipolygon *g1,
                   const Cartesian_polygon *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_multipolygon *g1,
                   const Cartesian_geometrycollection *g2) const {
  return geometry_collection_apply_touches<Cartesian_geometrycollection>(
      *this, g1, g2);
}

bool Touches::eval(const Cartesian_multipolygon *g1,
                   const Cartesian_multipoint *g2) const {
  return eval(g2, g1);
}

bool Touches::eval(const Cartesian_multipolygon *g1,
                   const Cartesian_multilinestring *g2) const {
  return bg::touches(*g1, *g2);
}

bool Touches::eval(const Cartesian_multipolygon *g1,
                   const Cartesian_multipolygon *g2) const {
  return bg::touches(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// touches(Geographic_point, *)

bool Touches::eval(const Geographic_point *, const Geographic_point *) const {
  // If dim(g1) == 0 and dim(g2) == 0, return NULL (SQL/MM 2015 Part 3,
  // Sect. 5.1.50).
  throw null_value_exception();
}

bool Touches::eval(const Geographic_point *g1,
                   const Geographic_linestring *g2) const {
  return bg::touches(*g1, *g2, m_geographic_pl_pa_strategy);
}

bool Touches::eval(const Geographic_point *g1,
                   const Geographic_polygon *g2) const {
  return bg::touches(*g1, *g2, m_geographic_pl_pa_strategy);
}

bool Touches::eval(const Geographic_point *g1,
                   const Geographic_geometrycollection *g2) const {
  return geometry_collection_apply_touches<Geographic_geometrycollection>(
      *this, g1, g2);
}

bool Touches::eval(const Geographic_point *,
                   const Geographic_multipoint *) const {
  // If dim(g1) == 0 and dim(g2) == 0, return NULL (SQL/MM 2015 Part 3,
  // Sect. 5.1.50).
  throw null_value_exception();
}

bool Touches::eval(const Geographic_point *g1,
                   const Geographic_multilinestring *g2) const {
  return bg::touches(*g1, *g2, m_geographic_pl_pa_strategy);
}

bool Touches::eval(const Geographic_point *g1,
                   const Geographic_multipolygon *g2) const {
  return bg::touches(*g1, *g2, m_geographic_pl_pa_strategy);
}

//////////////////////////////////////////////////////////////////////////////

// touches(Geographic_linestring, *)

bool Touches::eval(const Geographic_linestring *g1,
                   const Geographic_point *g2) const {
  return bg::touches(*g1, *g2, m_geographic_pl_pa_strategy);
}

bool Touches::eval(const Geographic_linestring *g1,
                   const Geographic_linestring *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Touches::eval(const Geographic_linestring *g1,
                   const Geographic_polygon *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Touches::eval(const Geographic_linestring *g1,
                   const Geographic_geometrycollection *g2) const {
  return geometry_collection_apply_touches<Geographic_geometrycollection>(
      *this, g1, g2);
}

bool Touches::eval(const Geographic_linestring *g1,
                   const Geographic_multipoint *g2) const {
  return eval(g2, g1);
}

bool Touches::eval(const Geographic_linestring *g1,
                   const Geographic_multilinestring *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Touches::eval(const Geographic_linestring *g1,
                   const Geographic_multipolygon *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

//////////////////////////////////////////////////////////////////////////////

// touches(Geographic_polygon, *)

bool Touches::eval(const Geographic_polygon *g1,
                   const Geographic_point *g2) const {
  return bg::touches(*g1, *g2, m_geographic_pl_pa_strategy);
}

bool Touches::eval(const Geographic_polygon *g1,
                   const Geographic_linestring *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Touches::eval(const Geographic_polygon *g1,
                   const Geographic_polygon *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Touches::eval(const Geographic_polygon *g1,
                   const Geographic_geometrycollection *g2) const {
  return geometry_collection_apply_touches<Geographic_geometrycollection>(
      *this, g1, g2);
}

bool Touches::eval(const Geographic_polygon *g1,
                   const Geographic_multipoint *g2) const {
  return eval(g2, g1);
}

bool Touches::eval(const Geographic_polygon *g1,
                   const Geographic_multilinestring *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Touches::eval(const Geographic_polygon *g1,
                   const Geographic_multipolygon *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

//////////////////////////////////////////////////////////////////////////////

// touches(Geographic_geometrycollection, *)

bool Touches::eval(const Geographic_geometrycollection *g1,
                   const Geometry *g2) const {
  return geometry_collection_apply_touches<Geographic_geometrycollection>(
      *this, g1, g2);
}

//////////////////////////////////////////////////////////////////////////////

// touches(Geographic_multipoint, *)

bool Touches::eval(const Geographic_multipoint *,
                   const Geographic_point *) const {
  // If dim(g1) == 0 and dim(g2) == 0, return NULL (SQL/MM 2015 Part 3,
  // Sect. 5.1.50).
  throw null_value_exception();
}

bool Touches::eval(const Geographic_multipoint *g1,
                   const Geographic_linestring *g2) const {
  Within within(m_semi_major, m_semi_minor);
  bool touches = false;

  // At least one point in g1 has to touch g2, and none of the points in g1
  // may be within g2.
  for (auto &pt : *g1) {
    bool pt_touches = false;
    if (!touches) {
      pt_touches = bg::touches(pt, *g2, m_geographic_pl_pa_strategy);
      touches = pt_touches;
    }
    if (!pt_touches) {
      if (within(&pt, g2)) return false;
    }
  }

  return touches;
}

bool Touches::eval(const Geographic_multipoint *g1,
                   const Geographic_polygon *g2) const {
  Within within(m_semi_major, m_semi_minor);
  bool touches = false;

  // At least one point in g1 has to touch g2, and none of the points in g1
  // may be within g2.
  for (auto &pt : *g1) {
    bool pt_touches = false;
    if (!touches) {
      pt_touches = bg::touches(pt, *g2, m_geographic_pl_pa_strategy);
      touches = pt_touches;
    }
    if (!pt_touches) {
      if (within(&pt, g2)) return false;
    }
  }

  return touches;
}

bool Touches::eval(const Geographic_multipoint *g1,
                   const Geographic_geometrycollection *g2) const {
  return geometry_collection_apply_touches<Geographic_geometrycollection>(
      *this, g1, g2);
}

bool Touches::eval(const Geographic_multipoint *,
                   const Geographic_multipoint *) const {
  // If dim(g1) == 0 and dim(g2) == 0, return NULL (SQL/MM 2015 Part 3,
  // Sect. 5.1.50).
  throw null_value_exception();
}

bool Touches::eval(const Geographic_multipoint *g1,
                   const Geographic_multilinestring *g2) const {
  Within within(m_semi_major, m_semi_minor);
  bool touches = false;

  // At least one point in g1 has to touch g2, and none of the points in g1
  // may be within g2.
  for (auto &pt : *g1) {
    bool pt_touches = false;
    if (!touches) {
      pt_touches = bg::touches(pt, *g2, m_geographic_pl_pa_strategy);
      touches = pt_touches;
    }
    if (!pt_touches) {
      if (within(&pt, g2)) return false;
    }
  }

  return touches;
}

bool Touches::eval(const Geographic_multipoint *g1,
                   const Geographic_multipolygon *g2) const {
  Within within(m_semi_major, m_semi_minor);
  bool touches = false;

  // At least one point in g1 has to touch g2, and none of the points in g1
  // may be within g2.
  for (auto &pt : *g1) {
    bool pt_touches = false;
    if (!touches) {
      pt_touches = bg::touches(pt, *g2, m_geographic_pl_pa_strategy);
      touches = pt_touches;
    }
    if (!pt_touches) {
      if (within(&pt, g2)) return false;
    }
  }

  return touches;
}

//////////////////////////////////////////////////////////////////////////////

// touches(Geographic_multilinestring, *)

bool Touches::eval(const Geographic_multilinestring *g1,
                   const Geographic_point *g2) const {
  return bg::touches(*g1, *g2, m_geographic_pl_pa_strategy);
}

bool Touches::eval(const Geographic_multilinestring *g1,
                   const Geographic_linestring *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Touches::eval(const Geographic_multilinestring *g1,
                   const Geographic_polygon *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Touches::eval(const Geographic_multilinestring *g1,
                   const Geographic_geometrycollection *g2) const {
  return geometry_collection_apply_touches<Geographic_geometrycollection>(
      *this, g1, g2);
}

bool Touches::eval(const Geographic_multilinestring *g1,
                   const Geographic_multipoint *g2) const {
  return eval(g2, g1);
}

bool Touches::eval(const Geographic_multilinestring *g1,
                   const Geographic_multilinestring *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Touches::eval(const Geographic_multilinestring *g1,
                   const Geographic_multipolygon *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

//////////////////////////////////////////////////////////////////////////////

// touches(Geographic_multipolygon, *)

bool Touches::eval(const Geographic_multipolygon *g1,
                   const Geographic_point *g2) const {
  return bg::touches(*g1, *g2, m_geographic_pl_pa_strategy);
}

bool Touches::eval(const Geographic_multipolygon *g1,
                   const Geographic_linestring *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Touches::eval(const Geographic_multipolygon *g1,
                   const Geographic_polygon *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Touches::eval(const Geographic_multipolygon *g1,
                   const Geographic_geometrycollection *g2) const {
  return geometry_collection_apply_touches<Geographic_geometrycollection>(
      *this, g1, g2);
}

bool Touches::eval(const Geographic_multipolygon *g1,
                   const Geographic_multipoint *g2) const {
  return eval(g2, g1);
}

bool Touches::eval(const Geographic_multipolygon *g1,
                   const Geographic_multilinestring *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Touches::eval(const Geographic_multipolygon *g1,
                   const Geographic_multipolygon *g2) const {
  return bg::touches(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

//////////////////////////////////////////////////////////////////////////////

// equals(Box, Box)

bool Touches::eval(const Cartesian_box *b1, const Cartesian_box *b2) const {
  if (mbr_is_point(*b1)) {
    if (mbr_is_point(*b2)) {
      // For two geometries to touch, the interior must not intersect. If
      // g1 and g2 are points, the MBRs will either be disjoint or
      // equal. Hence, point-point mbr_touches is always false.
      return false;
    }

    if (mbr_is_line(*b2)) {
      return (b1->min_corner().x() == b2->min_corner().x() &&
              b1->min_corner().y() == b2->min_corner().y()) ||
             (b1->min_corner().x() == b2->max_corner().x() &&
              b1->min_corner().y() == b2->max_corner().y());
    }

    return bg::touches(*b1, *b2);
  }

  if (mbr_is_line(*b1)) {
    if (mbr_is_point(*b2)) {
      return (b2->min_corner().x() == b1->min_corner().x() &&
              b2->min_corner().y() == b1->min_corner().y()) ||
             (b2->min_corner().x() == b1->max_corner().x() &&
              b2->min_corner().y() == b1->max_corner().y());
    }

    if (mbr_is_line(*b2)) {
      Cartesian_point b1_ls_start(b1->min_corner().x(), b1->min_corner().y());
      Cartesian_point b1_ls_end(b1->max_corner().x(), b1->max_corner().y());
      Cartesian_linestring b1_ls;
      b1_ls.push_back(b1_ls_start);
      b1_ls.push_back(b1_ls_end);

      Cartesian_point b2_ls_start(b2->min_corner().x(), b2->min_corner().y());
      Cartesian_point b2_ls_end(b2->max_corner().x(), b2->max_corner().y());
      Cartesian_linestring b2_ls;
      b2_ls.push_back(b2_ls_start);
      b2_ls.push_back(b2_ls_end);

      return eval(&b1_ls, &b2_ls);
    }

    return bg::touches(*b1, *b2);
  }

  return bg::touches(b1, b2);
}

bool Touches::eval(const Geographic_box *b1, const Geographic_box *b2) const {
  if (mbr_is_point(*b1)) {
    if (mbr_is_point(*b2)) {
      // For two geometries to touch, the interior must not intersect. If
      // g1 and g2 are points, the MBRs will either be disjoint or
      // equal. Hence, point-point mbr_touches is always false.
      return false;
    }

    if (mbr_is_line(*b2)) {
      return (b1->min_corner().x() == b2->min_corner().x() &&
              b1->min_corner().y() == b2->min_corner().y()) ||
             (b1->min_corner().x() == b2->max_corner().x() &&
              b1->min_corner().y() == b2->max_corner().y());
    }

    return bg::touches(*b1, *b2);
  }

  if (mbr_is_line(*b1)) {
    if (mbr_is_point(*b2)) {
      return (b2->min_corner().x() == b1->min_corner().x() &&
              b2->min_corner().y() == b1->min_corner().y()) ||
             (b2->min_corner().x() == b1->max_corner().x() &&
              b2->min_corner().y() == b1->max_corner().y());
    }

    if (mbr_is_line(*b2)) {
      Geographic_point b1_ls_start(b1->min_corner().x(), b1->min_corner().y());
      Geographic_point b1_ls_end(b1->max_corner().x(), b1->max_corner().y());
      Geographic_linestring b1_ls;
      b1_ls.push_back(b1_ls_start);
      b1_ls.push_back(b1_ls_end);

      Geographic_point b2_ls_start(b2->min_corner().x(), b2->min_corner().y());
      Geographic_point b2_ls_end(b2->max_corner().x(), b2->max_corner().y());
      Geographic_linestring b2_ls;
      b2_ls.push_back(b2_ls_start);
      b2_ls.push_back(b2_ls_end);

      return eval(&b1_ls, &b2_ls);
    }

    return bg::touches(*b1, *b2);
  }

  return bg::touches(*b1, *b2);
}

//////////////////////////////////////////////////////////////////////////////

bool touches(const dd::Spatial_reference_system *srs, const Geometry *g1,
             const Geometry *g2, const char *func_name, bool *touches,
             bool *null) noexcept {
  try {
    DBUG_ASSERT(g1->coordinate_system() == g2->coordinate_system());
    DBUG_ASSERT(srs == nullptr ||
                ((srs->is_cartesian() &&
                  g1->coordinate_system() == Coordinate_system::kCartesian) ||
                 (srs->is_geographic() &&
                  g1->coordinate_system() == Coordinate_system::kGeographic)));

    if ((*null = (g1->is_empty() || g2->is_empty()))) return false;

    Touches touches_func(srs ? srs->semi_major_axis() : 0.0,
                         srs ? srs->semi_minor_axis() : 0.0);
    *touches = touches_func(g1, g2);
  } catch (const null_value_exception &) {
    *null = true;
    return false;
  } catch (...) {
    handle_gis_exception(func_name);
    return true;
  }

  return false;
}

bool mbr_touches(const dd::Spatial_reference_system *srs, const Geometry *g1,
                 const Geometry *g2, const char *func_name, bool *touches,
                 bool *null) noexcept {
  try {
    DBUG_ASSERT(g1->coordinate_system() == g2->coordinate_system());
    DBUG_ASSERT(srs == nullptr ||
                ((srs->is_cartesian() &&
                  g1->coordinate_system() == Coordinate_system::kCartesian) ||
                 (srs->is_geographic() &&
                  g1->coordinate_system() == Coordinate_system::kGeographic)));

    if ((*null = (g1->is_empty() || g2->is_empty()))) return false;

    Touches touches_func(srs ? srs->semi_major_axis() : 0.0,
                         srs ? srs->semi_minor_axis() : 0.0);

    switch (g1->coordinate_system()) {
      case Coordinate_system::kCartesian: {
        Cartesian_box mbr1;
        box_envelope(g1, srs, &mbr1);
        Cartesian_box mbr2;
        box_envelope(g2, srs, &mbr2);
        *touches = touches_func(&mbr1, &mbr2);
        break;
      }
      case Coordinate_system::kGeographic: {
        Geographic_box mbr1;
        box_envelope(g1, srs, &mbr1);
        Geographic_box mbr2;
        box_envelope(g2, srs, &mbr2);
        *touches = touches_func(&mbr1, &mbr2);
        break;
      }
    }
  } catch (...) {
    handle_gis_exception(func_name);
    return true;
  }

  return false;
}

}  // namespace gis
