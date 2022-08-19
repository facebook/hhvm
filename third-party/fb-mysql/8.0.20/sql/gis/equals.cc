// Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
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
/// This file implements the equals functor and function.

#include <memory>  // std::unique_ptr

#include <boost/geometry.hpp>

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/box.h"
#include "sql/gis/box_traits.h"
#include "sql/gis/equals_functor.h"
#include "sql/gis/gc_utils.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_traits.h"
#include "sql/gis/mbr_utils.h"
#include "sql/gis/relops.h"
#include "sql/sql_exception_handler.h"  // handle_gis_exception
#include "template_utils.h"             // down_cast

namespace bg = boost::geometry;

namespace gis {

/// Apply an Equals functor to two geometries, which both may be geometry
/// collections, and return the boolean result of the functor applied on each
/// combination of elements in the collections.
///
/// @tparam GC Coordinate specific gometry collection type.
///
/// @param f Functor to apply.
/// @param g1 First geometry.
/// @param g2 Second geometry.
///
/// @retval true g1 equals g2.
/// @retval false g1 doesn't equal g2.
template <typename GC>
static bool geometry_collection_apply_equals(const Equals &f,
                                             const Geometry *g1,
                                             const Geometry *g2) {
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

      if (g1_mpt->empty() != g2_mpt->empty() ||
          g1_mls->empty() != g2_mls->empty() ||
          g1_mpy->empty() != g2_mpy->empty())
        return false;

      bool mpt_equals;
      if (g1_mpt->empty() && g2_mpt->empty())
        mpt_equals = true;
      else
        mpt_equals = f(g1_mpt.get(), g2_mpt.get());

      bool mls_equals;
      if (g1_mls->empty() && g2_mls->empty())
        mls_equals = true;
      else
        mls_equals = f(g1_mls.get(), g2_mls.get());

      bool mpy_equals;
      if (g1_mpy->empty() && g2_mpy->empty())
        mpy_equals = true;
      else
        mpy_equals = f(g1_mpy.get(), g2_mpy.get());

      return mpt_equals && mls_equals && mpy_equals;
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

      switch (g1->type()) {
        case Geometry_type::kPoint:
        case Geometry_type::kMultipoint: {
          bool mls_empty = !g2_mls.get() || g2_mls->empty();
          bool mpy_empty = !g2_mpy.get() || g2_mpy->empty();
          if (!mls_empty || !mpy_empty) return false;
          return f(g1, g2_mpt.get());
        }
        case Geometry_type::kLinestring:
        case Geometry_type::kMultilinestring: {
          bool mpt_empty = !g2_mpt.get() || g2_mpt->empty();
          bool mpy_empty = !g2_mpy.get() || g2_mpy->empty();
          if (!mpt_empty || !mpy_empty) return false;
          return f(g1, g2_mls.get());
        }
        case Geometry_type::kPolygon:
        case Geometry_type::kMultipolygon: {
          bool mpt_empty = !g2_mpt.get() || g2_mpt->empty();
          bool mls_empty = !g2_mls.get() || g2_mls->empty();
          if (!mpt_empty || !mls_empty) return false;
          return f(g1, g2_mpy.get());
        }
        default:
          // All possible combinations should be covered above.
          DBUG_ASSERT(false);
          return false;
      }
    } else {
      return f(g1, g2);
    }
  }
}

Equals::Equals(double semi_major, double semi_minor)
    : m_semi_major(semi_major),
      m_semi_minor(semi_minor),
      m_geographic_ll_aa_strategy(
          bg::srs::spheroid<double>(semi_major, semi_minor)) {}

bool Equals::operator()(const Geometry *g1, const Geometry *g2) const {
  return apply(*this, g1, g2);
}

bool Equals::operator()(const Box *b1, const Box *b2) const {
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

bool Equals::eval(const Geometry *g1, const Geometry *g2) const {
  // All parameter type combinations have been implemented.
  DBUG_ASSERT(false);
  throw not_implemented_exception::for_non_projected(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// equals(Cartesian_point, *)

bool Equals::eval(const Cartesian_point *g1, const Cartesian_point *g2) const {
  return bg::equals(*g1, *g2);
}

bool Equals::eval(const Cartesian_point *, const Cartesian_linestring *) const {
  // A point may never be equal to a linestring.
  return false;
}

bool Equals::eval(const Cartesian_point *, const Cartesian_polygon *) const {
  // A point may never be equal to a polygon.
  return false;
}

bool Equals::eval(const Cartesian_point *g1,
                  const Cartesian_geometrycollection *g2) const {
  return geometry_collection_apply_equals<Cartesian_geometrycollection>(*this,
                                                                        g1, g2);
}

bool Equals::eval(const Cartesian_point *g1,
                  const Cartesian_multipoint *g2) const {
  return bg::equals(*g1, *g2);
}

bool Equals::eval(const Cartesian_point *,
                  const Cartesian_multilinestring *) const {
  // A point may never be equal to a multilinestring.
  return false;
}

bool Equals::eval(const Cartesian_point *,
                  const Cartesian_multipolygon *) const {
  // A point may never be equal to a multipolygon.
  return false;
}

//////////////////////////////////////////////////////////////////////////////

// equals(Cartesian_linestring, *)

bool Equals::eval(const Cartesian_linestring *, const Cartesian_point *) const {
  // A linestring may never be equal to a point.
  return false;
}

bool Equals::eval(const Cartesian_linestring *g1,
                  const Cartesian_linestring *g2) const {
  return bg::equals(*g1, *g2);
}

bool Equals::eval(const Cartesian_linestring *,
                  const Cartesian_polygon *) const {
  // A linestring may never be equal to a polygon.
  return false;
}

bool Equals::eval(const Cartesian_linestring *g1,
                  const Cartesian_geometrycollection *g2) const {
  return geometry_collection_apply_equals<Cartesian_geometrycollection>(*this,
                                                                        g1, g2);
}

bool Equals::eval(const Cartesian_linestring *,
                  const Cartesian_multipoint *) const {
  // A linestring may never be equal to a multipoint.
  return false;
}

bool Equals::eval(const Cartesian_linestring *g1,
                  const Cartesian_multilinestring *g2) const {
  return bg::equals(*g1, *g2);
}

bool Equals::eval(const Cartesian_linestring *,
                  const Cartesian_multipolygon *) const {
  // A linestring may never be equal to a multipolygon.
  return false;
}

//////////////////////////////////////////////////////////////////////////////

// equals(Cartesian_polygon, *)

bool Equals::eval(const Cartesian_polygon *, const Cartesian_point *) const {
  // A polygon may never be equal to a point.
  return false;
}

bool Equals::eval(const Cartesian_polygon *,
                  const Cartesian_linestring *) const {
  // A polygon may never be equal to a linestring.
  return false;
}

bool Equals::eval(const Cartesian_polygon *g1,
                  const Cartesian_polygon *g2) const {
  return bg::equals(*g1, *g2);
}

bool Equals::eval(const Cartesian_polygon *g1,
                  const Cartesian_geometrycollection *g2) const {
  return geometry_collection_apply_equals<Cartesian_geometrycollection>(*this,
                                                                        g1, g2);
}

bool Equals::eval(const Cartesian_polygon *,
                  const Cartesian_multipoint *) const {
  // A polygon may never be equal to a multipoint.
  return false;
}

bool Equals::eval(const Cartesian_polygon *,
                  const Cartesian_multilinestring *) const {
  // A polygon may never be equal to a multilinestring.
  return false;
}

bool Equals::eval(const Cartesian_polygon *g1,
                  const Cartesian_multipolygon *g2) const {
  return bg::equals(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// equals(Cartesian_geometrycollection, *)

bool Equals::eval(const Cartesian_geometrycollection *g1,
                  const Geometry *g2) const {
  return geometry_collection_apply_equals<Cartesian_geometrycollection>(*this,
                                                                        g1, g2);
}

//////////////////////////////////////////////////////////////////////////////

// equals(Cartesian_multipoint, *)

bool Equals::eval(const Cartesian_multipoint *g1,
                  const Cartesian_point *g2) const {
  return eval(g2, g1);
}

bool Equals::eval(const Cartesian_multipoint *,
                  const Cartesian_linestring *) const {
  // A multipoint may never be equal to a linestring.
  return false;
}

bool Equals::eval(const Cartesian_multipoint *,
                  const Cartesian_polygon *) const {
  // A multipoint may never be equal to a polygon.
  return false;
}

bool Equals::eval(const Cartesian_multipoint *g1,
                  const Cartesian_geometrycollection *g2) const {
  return geometry_collection_apply_equals<Cartesian_geometrycollection>(*this,
                                                                        g1, g2);
}

bool Equals::eval(const Cartesian_multipoint *g1,
                  const Cartesian_multipoint *g2) const {
  return bg::equals(*g1, *g2);
}

bool Equals::eval(const Cartesian_multipoint *,
                  const Cartesian_multilinestring *) const {
  // A multipoint may never be equal to a multilinestring.
  return false;
}

bool Equals::eval(const Cartesian_multipoint *,
                  const Cartesian_multipolygon *) const {
  // A multipoint may never be equal to a multipolygon.
  return false;
}

//////////////////////////////////////////////////////////////////////////////

// equals(Cartesian_multilinestring, *)

bool Equals::eval(const Cartesian_multilinestring *,
                  const Cartesian_point *) const {
  // A multilinestring may never be equal to a point.
  return false;
}

bool Equals::eval(const Cartesian_multilinestring *g1,
                  const Cartesian_linestring *g2) const {
  return bg::equals(*g1, *g2);
}

bool Equals::eval(const Cartesian_multilinestring *,
                  const Cartesian_polygon *) const {
  // A multilinestring may never be equal to a polygon.
  return false;
}

bool Equals::eval(const Cartesian_multilinestring *g1,
                  const Cartesian_geometrycollection *g2) const {
  return geometry_collection_apply_equals<Cartesian_geometrycollection>(*this,
                                                                        g1, g2);
}

bool Equals::eval(const Cartesian_multilinestring *,
                  const Cartesian_multipoint *) const {
  // A multilinestring may never be equal to a multipoint.
  return false;
}

bool Equals::eval(const Cartesian_multilinestring *g1,
                  const Cartesian_multilinestring *g2) const {
  return bg::equals(*g1, *g2);
}

bool Equals::eval(const Cartesian_multilinestring *,
                  const Cartesian_multipolygon *) const {
  // A multilinestring may never be equal to a multipolygon.
  return false;
}

//////////////////////////////////////////////////////////////////////////////

// equals(Cartesian_multipolygon, *)

bool Equals::eval(const Cartesian_multipolygon *,
                  const Cartesian_point *) const {
  // A multipolygon may never be equal to a point.
  return false;
}

bool Equals::eval(const Cartesian_multipolygon *,
                  const Cartesian_linestring *) const {
  // A multipolygon may never be equal to a linestring.
  return false;
}

bool Equals::eval(const Cartesian_multipolygon *g1,
                  const Cartesian_polygon *g2) const {
  return bg::equals(*g1, *g2);
}

bool Equals::eval(const Cartesian_multipolygon *g1,
                  const Cartesian_geometrycollection *g2) const {
  return geometry_collection_apply_equals<Cartesian_geometrycollection>(*this,
                                                                        g1, g2);
}

bool Equals::eval(const Cartesian_multipolygon *,
                  const Cartesian_multipoint *) const {
  // A multipolygon may never be equal to a multipoint.
  return false;
}

bool Equals::eval(const Cartesian_multipolygon *,
                  const Cartesian_multilinestring *) const {
  // A multipolygon may never be equal to a multilinestring.
  return false;
}

bool Equals::eval(const Cartesian_multipolygon *g1,
                  const Cartesian_multipolygon *g2) const {
  return bg::equals(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// equals(Geographic_point, *)

bool Equals::eval(const Geographic_point *g1,
                  const Geographic_point *g2) const {
  // Default strategy is OK. P/P computations do not depend on shape of
  // ellipsoid.
  return bg::equals(*g1, *g2);
}

bool Equals::eval(const Geographic_point *,
                  const Geographic_linestring *) const {
  // A point may never be equal to a linestring.
  return false;
}

bool Equals::eval(const Geographic_point *, const Geographic_polygon *) const {
  // A point may never be equal to a polygon.
  return false;
}

bool Equals::eval(const Geographic_point *g1,
                  const Geographic_geometrycollection *g2) const {
  return geometry_collection_apply_equals<Geographic_geometrycollection>(
      *this, g1, g2);
}

bool Equals::eval(const Geographic_point *g1,
                  const Geographic_multipoint *g2) const {
  // Default strategy is OK. P/P computations do not depend on shape of
  // ellipsoid.
  return bg::equals(*g1, *g2);
}

bool Equals::eval(const Geographic_point *,
                  const Geographic_multilinestring *) const {
  // A point may never be equal to a multilinestring.
  return false;
}

bool Equals::eval(const Geographic_point *,
                  const Geographic_multipolygon *) const {
  // A point may never be equal to a multipolygon.
  return false;
}

//////////////////////////////////////////////////////////////////////////////

// equals(Geographic_linestring, *)

bool Equals::eval(const Geographic_linestring *,
                  const Geographic_point *) const {
  // A linestring may never be equal to a point.
  return false;
}

bool Equals::eval(const Geographic_linestring *g1,
                  const Geographic_linestring *g2) const {
  return bg::equals(*g1, *g2, m_geographic_ll_aa_strategy);
}

bool Equals::eval(const Geographic_linestring *,
                  const Geographic_polygon *) const {
  // A linestring may never be equal to a polygon.
  return false;
}

bool Equals::eval(const Geographic_linestring *g1,
                  const Geographic_geometrycollection *g2) const {
  return geometry_collection_apply_equals<Geographic_geometrycollection>(
      *this, g1, g2);
}

bool Equals::eval(const Geographic_linestring *,
                  const Geographic_multipoint *) const {
  // A linestring may never be equal to a multipoint.
  return false;
}

bool Equals::eval(const Geographic_linestring *g1,
                  const Geographic_multilinestring *g2) const {
  return bg::equals(*g1, *g2, m_geographic_ll_aa_strategy);
}

bool Equals::eval(const Geographic_linestring *,
                  const Geographic_multipolygon *) const {
  // A linestring may never be equal to a multipolygon.
  return false;
}

//////////////////////////////////////////////////////////////////////////////

// equals(Geographic_polygon, *)

bool Equals::eval(const Geographic_polygon *, const Geographic_point *) const {
  // A polygon may never be equal to a point.
  return false;
}

bool Equals::eval(const Geographic_polygon *,
                  const Geographic_linestring *) const {
  // A polygon may never be equal to a linestring.
  return false;
}

bool Equals::eval(const Geographic_polygon *g1,
                  const Geographic_polygon *g2) const {
  return bg::equals(*g1, *g2, m_geographic_ll_aa_strategy);
}

bool Equals::eval(const Geographic_polygon *g1,
                  const Geographic_geometrycollection *g2) const {
  return geometry_collection_apply_equals<Geographic_geometrycollection>(
      *this, g1, g2);
}

bool Equals::eval(const Geographic_polygon *,
                  const Geographic_multipoint *) const {
  // A polygon may never be equal to a multipoint.
  return false;
}

bool Equals::eval(const Geographic_polygon *,
                  const Geographic_multilinestring *) const {
  // A polygon may never be equal to a multilinestring.
  return false;
}

bool Equals::eval(const Geographic_polygon *g1,
                  const Geographic_multipolygon *g2) const {
  return bg::equals(*g1, *g2, m_geographic_ll_aa_strategy);
}

//////////////////////////////////////////////////////////////////////////////

// equals(Geographic_geometrycollection, *)

bool Equals::eval(const Geographic_geometrycollection *g1,
                  const Geometry *g2) const {
  return geometry_collection_apply_equals<Geographic_geometrycollection>(
      *this, g1, g2);
}

//////////////////////////////////////////////////////////////////////////////

// equals(Geographic_multipoint, *)

bool Equals::eval(const Geographic_multipoint *g1,
                  const Geographic_point *g2) const {
  return eval(g2, g1);
}

bool Equals::eval(const Geographic_multipoint *,
                  const Geographic_linestring *) const {
  // A multipoint may never be equal to a linestring.
  return false;
}

bool Equals::eval(const Geographic_multipoint *,
                  const Geographic_polygon *) const {
  // A multipoint may never be equal to a polygon.
  return false;
}

bool Equals::eval(const Geographic_multipoint *g1,
                  const Geographic_geometrycollection *g2) const {
  return geometry_collection_apply_equals<Geographic_geometrycollection>(
      *this, g1, g2);
}

bool Equals::eval(const Geographic_multipoint *g1,
                  const Geographic_multipoint *g2) const {
  // Default strategy is OK. P/P computations do not depend on shape of
  // ellipsoid.
  return bg::equals(*g1, *g2);
}

bool Equals::eval(const Geographic_multipoint *,
                  const Geographic_multilinestring *) const {
  // A multipoint may never be equal to a multilinestring.
  return false;
}

bool Equals::eval(const Geographic_multipoint *,
                  const Geographic_multipolygon *) const {
  // A multipoint may never be equal to a multipolygon.
  return false;
}

//////////////////////////////////////////////////////////////////////////////

// equals(Geographic_multilinestring, *)

bool Equals::eval(const Geographic_multilinestring *,
                  const Geographic_point *) const {
  // A multilinestring may never be equal to a point.
  return false;
}

bool Equals::eval(const Geographic_multilinestring *g1,
                  const Geographic_linestring *g2) const {
  return bg::equals(*g1, *g2, m_geographic_ll_aa_strategy);
}

bool Equals::eval(const Geographic_multilinestring *,
                  const Geographic_polygon *) const {
  // A multilinestring may never be equal to a polygon.
  return false;
}

bool Equals::eval(const Geographic_multilinestring *g1,
                  const Geographic_geometrycollection *g2) const {
  return geometry_collection_apply_equals<Geographic_geometrycollection>(
      *this, g1, g2);
}

bool Equals::eval(const Geographic_multilinestring *,
                  const Geographic_multipoint *) const {
  // A multilinestring may never be equal to a multipoint.
  return false;
}

bool Equals::eval(const Geographic_multilinestring *g1,
                  const Geographic_multilinestring *g2) const {
  return bg::equals(*g1, *g2, m_geographic_ll_aa_strategy);
}

bool Equals::eval(const Geographic_multilinestring *,
                  const Geographic_multipolygon *) const {
  // A multilinestring may never be equal to a multipolygon.
  return false;
}

//////////////////////////////////////////////////////////////////////////////

// equals(Geographic_multipolygon, *)

bool Equals::eval(const Geographic_multipolygon *,
                  const Geographic_point *) const {
  // A multipolygon may never be equal to a point.
  return false;
}

bool Equals::eval(const Geographic_multipolygon *,
                  const Geographic_linestring *) const {
  // A multipolygon may never be equal to a linestring.
  return false;
}

bool Equals::eval(const Geographic_multipolygon *g1,
                  const Geographic_polygon *g2) const {
  return bg::equals(*g1, *g2, m_geographic_ll_aa_strategy);
}

bool Equals::eval(const Geographic_multipolygon *g1,
                  const Geographic_geometrycollection *g2) const {
  return geometry_collection_apply_equals<Geographic_geometrycollection>(
      *this, g1, g2);
}

bool Equals::eval(const Geographic_multipolygon *,
                  const Geographic_multipoint *) const {
  // A multipolygon may never be equal to a multipoint.
  return false;
}

bool Equals::eval(const Geographic_multipolygon *,
                  const Geographic_multilinestring *) const {
  // A multipolygon may never be equal to a multilinestring.
  return false;
}

bool Equals::eval(const Geographic_multipolygon *g1,
                  const Geographic_multipolygon *g2) const {
  return bg::equals(*g1, *g2, m_geographic_ll_aa_strategy);
}

//////////////////////////////////////////////////////////////////////////////

// equals(Box, Box)

bool Equals::eval(const Cartesian_box *b1, const Cartesian_box *b2) const {
  return bg::equals(*b1, *b2);
}

bool Equals::eval(const Geographic_box *b1, const Geographic_box *b2) const {
  return bg::equals(*b1, *b2);
}

//////////////////////////////////////////////////////////////////////////////

bool equals(const dd::Spatial_reference_system *srs, const Geometry *g1,
            const Geometry *g2, const char *func_name, bool *equals,
            bool *null) noexcept {
  try {
    DBUG_ASSERT(g1->coordinate_system() == g2->coordinate_system());
    DBUG_ASSERT(srs == nullptr ||
                ((srs->is_cartesian() &&
                  g1->coordinate_system() == Coordinate_system::kCartesian) ||
                 (srs->is_geographic() &&
                  g1->coordinate_system() == Coordinate_system::kGeographic)));

    *null = false;
    if (g1->is_empty()) {
      *equals = g2->is_empty();
      return false;
    }
    if (g2->is_empty()) {
      *equals = false;
      return false;
    }

    Equals equals_func(srs ? srs->semi_major_axis() : 0.0,
                       srs ? srs->semi_minor_axis() : 0.0);
    *equals = equals_func(g1, g2);
  } catch (...) {
    handle_gis_exception(func_name);
    return true;
  }

  return false;
}

bool mbr_equals(const dd::Spatial_reference_system *srs, const Geometry *g1,
                const Geometry *g2, const char *func_name, bool *equals,
                bool *null) noexcept {
  try {
    DBUG_ASSERT(g1->coordinate_system() == g2->coordinate_system());
    DBUG_ASSERT(srs == nullptr ||
                ((srs->is_cartesian() &&
                  g1->coordinate_system() == Coordinate_system::kCartesian) ||
                 (srs->is_geographic() &&
                  g1->coordinate_system() == Coordinate_system::kGeographic)));

    *null = false;
    if (g1->is_empty()) {
      *equals = g2->is_empty();
      return false;
    }
    if (g2->is_empty()) {
      *equals = false;
      return false;
    }

    Equals equals_func(srs ? srs->semi_major_axis() : 0.0,
                       srs ? srs->semi_minor_axis() : 0.0);

    switch (g1->coordinate_system()) {
      case Coordinate_system::kCartesian: {
        Cartesian_box mbr1;
        box_envelope(g1, srs, &mbr1);
        Cartesian_box mbr2;
        box_envelope(g2, srs, &mbr2);
        *equals = equals_func(&mbr1, &mbr2);
        break;
      }
      case Coordinate_system::kGeographic: {
        Geographic_box mbr1;
        box_envelope(g1, srs, &mbr1);
        Geographic_box mbr2;
        box_envelope(g2, srs, &mbr2);
        *equals = equals_func(&mbr1, &mbr2);
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
