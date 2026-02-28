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
/// This file implements the within functor and function.

#include <cmath>  // std::isfinite
#include <limits>
#include <memory>  // std::unique_ptr

#include <boost/geometry.hpp>

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/box.h"
#include "sql/gis/box_traits.h"
#include "sql/gis/difference_functor.h"
#include "sql/gis/equals_functor.h"
#include "sql/gis/gc_utils.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_traits.h"
#include "sql/gis/intersects_functor.h"
#include "sql/gis/mbr_utils.h"
#include "sql/gis/relops.h"
#include "sql/gis/within_functor.h"
#include "sql/sql_exception_handler.h"  // handle_gis_exception
#include "template_utils.h"             // down_cast

namespace bg = boost::geometry;

namespace gis {

Within::Within(double semi_major, double semi_minor)
    : m_semi_major(semi_major),
      m_semi_minor(semi_minor),
      m_geographic_pl_pa_strategy(
          bg::srs::spheroid<double>(semi_major, semi_minor)),
      m_geographic_ll_la_aa_strategy(
          bg::srs::spheroid<double>(semi_major, semi_minor)) {}

bool Within::operator()(const Geometry *g1, const Geometry *g2) const {
  return apply(*this, g1, g2);
}

bool Within::operator()(const Box *b1, const Box *b2) const {
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

bool Within::eval(const Geometry *g1, const Geometry *g2) const {
  // All parameter type combinations have been implemented.
  DBUG_ASSERT(false);
  throw not_implemented_exception::for_non_projected(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// within(Cartesian_point, *)

bool Within::eval(const Cartesian_point *g1, const Cartesian_point *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_point *g1,
                  const Cartesian_linestring *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_point *g1,
                  const Cartesian_polygon *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_point *g1,
                  const Cartesian_geometrycollection *g2) const {
  // g1 must be within one of the elements of g2.
  for (auto g : *g2)
    if ((*this)(g1, g)) return true;
  return false;
}

bool Within::eval(const Cartesian_point *g1,
                  const Cartesian_multipoint *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_point *g1,
                  const Cartesian_multilinestring *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_point *g1,
                  const Cartesian_multipolygon *g2) const {
  return bg::within(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// within(Cartesian_linestring, *)

bool Within::eval(const Cartesian_linestring *, const Cartesian_point *) const {
  // A linestring can never be within a point.
  return false;
}

bool Within::eval(const Cartesian_linestring *g1,
                  const Cartesian_linestring *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_linestring *g1,
                  const Cartesian_polygon *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_linestring *g1,
                  const Cartesian_geometrycollection *g2) const {
  // For g1 to be within g2, no point of g1 may be in the exterior of g2 and at
  // least one point of the interior of g1 has to be within the interior of g2.

  std::unique_ptr<Multipoint> g2_mpt;
  std::unique_ptr<Multilinestring> g2_mls;
  std::unique_ptr<Multipolygon> g2_mpy;
  split_gc(down_cast<const Geometrycollection *>(g2), &g2_mpt, &g2_mls,
           &g2_mpy);
  gc_union(m_semi_major, m_semi_minor, &g2_mpt, &g2_mls, &g2_mpy);

  Difference difference(m_semi_major, m_semi_minor);
  std::unique_ptr<Multilinestring> g1_diff_g2(new Cartesian_multilinestring());
  g1_diff_g2.reset(down_cast<Cartesian_multilinestring *>(
      difference(g1, down_cast<Cartesian_multilinestring *>(g2_mls.get()))));
  g1_diff_g2.reset(down_cast<Cartesian_multilinestring *>(
      difference(down_cast<Cartesian_multilinestring *>(g1_diff_g2.get()),
                 down_cast<Cartesian_multipolygon *>(g2_mpy.get()))));

  boost::geometry::de9im::mask mask("T********");
  return g1_diff_g2->empty() &&
         (bg::relate(*g1, *down_cast<Cartesian_multilinestring *>(g2_mls.get()),
                     mask) ||
          bg::relate(*g1, *down_cast<Cartesian_multipolygon *>(g2_mpy.get()),
                     mask));
}

bool Within::eval(const Cartesian_linestring *,
                  const Cartesian_multipoint *) const {
  // A linestring can never be within a multipoint.
  return false;
}

bool Within::eval(const Cartesian_linestring *g1,
                  const Cartesian_multilinestring *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_linestring *g1,
                  const Cartesian_multipolygon *g2) const {
  return bg::within(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// within(Cartesian_polygon, *)

bool Within::eval(const Cartesian_polygon *, const Cartesian_point *) const {
  // A polygon can never be within a point.
  return false;
}

bool Within::eval(const Cartesian_polygon *,
                  const Cartesian_linestring *) const {
  // A polygon can never be within a linestring.
  return false;
}

bool Within::eval(const Cartesian_polygon *g1,
                  const Cartesian_polygon *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_polygon *g1,
                  const Cartesian_geometrycollection *g2) const {
  // Polygons can only be within other polygons or multipolygons, so we can
  // ignore points and linestrings. g1 is within g2 if g1 is within the union
  // multipolygon of g2.
  std::unique_ptr<Multipoint> g2_mpt;
  std::unique_ptr<Multilinestring> g2_mls;
  std::unique_ptr<Multipolygon> g2_mpy;
  split_gc(down_cast<const Geometrycollection *>(g2), &g2_mpt, &g2_mls,
           &g2_mpy);
  gc_union(m_semi_major, m_semi_minor, &g2_mpt, &g2_mls, &g2_mpy);
  return eval(g1, down_cast<Cartesian_multipolygon *>(g2_mpy.get()));
}

bool Within::eval(const Cartesian_polygon *,
                  const Cartesian_multipoint *) const {
  // A polygon can never be within a multipoint.
  return false;
}

bool Within::eval(const Cartesian_polygon *,
                  const Cartesian_multilinestring *) const {
  // A polygon can never be within a multilinestring.
  return false;
}

bool Within::eval(const Cartesian_polygon *g1,
                  const Cartesian_multipolygon *g2) const {
  return bg::within(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// within(Cartesian_geometrycollection, *)

bool Within::eval(const Cartesian_geometrycollection *g1,
                  const Cartesian_point *g2) const {
  Equals equals(m_semi_major, m_semi_minor);
  return equals(g1, g2);
}

bool Within::eval(const Cartesian_geometrycollection *g1,
                  const Cartesian_linestring *g2) const {
  // g1 is within g2 if g1 contains only points and linestrings. One of the
  // elements of g1 must be within g2, the rest must be covered by g2.
  std::unique_ptr<Multipoint> g1_mpt;
  std::unique_ptr<Multilinestring> g1_mls;
  std::unique_ptr<Multipolygon> g1_mpy;
  split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
           &g1_mpy);
  gc_union(m_semi_major, m_semi_minor, &g1_mpt, &g1_mls, &g1_mpy);

  if (!g1_mpy->empty()) return false;

  if (eval(down_cast<Cartesian_multipoint *>(g1_mpt.get()), g2))
    return g1_mls->empty() ||
           bg::covered_by(*down_cast<Cartesian_multilinestring *>(g1_mls.get()),
                          *g2);
  if (eval(down_cast<Cartesian_multilinestring *>(g1_mls.get()), g2)) {
    for (auto &pt : *down_cast<Cartesian_multipoint *>(g1_mpt.get()))
      if (!bg::covered_by(pt, *g2)) return false;
    return true;
  }
  return false;
}

bool Within::eval(const Cartesian_geometrycollection *g1,
                  const Cartesian_polygon *g2) const {
  // At least one element of g1 has to be within g2. The rest have to be covered
  // by g2.
  std::unique_ptr<Multipoint> g1_mpt;
  std::unique_ptr<Multilinestring> g1_mls;
  std::unique_ptr<Multipolygon> g1_mpy;
  split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
           &g1_mpy);
  gc_union(m_semi_major, m_semi_minor, &g1_mpt, &g1_mls, &g1_mpy);

  if (eval(down_cast<Cartesian_multipoint *>(g1_mpt.get()), g2)) {
    return (g1_mls->empty() ||
            bg::covered_by(
                *down_cast<Cartesian_multilinestring *>(g1_mls.get()), *g2)) &&
           (g1_mpy->empty() ||
            bg::covered_by(*down_cast<Cartesian_multipolygon *>(g1_mpy.get()),
                           *g2));
  }
  if (eval(down_cast<Cartesian_multilinestring *>(g1_mls.get()), g2)) {
    for (auto &pt : *down_cast<Cartesian_multipoint *>(g1_mpt.get()))
      if (!bg::covered_by(pt, *g2)) return false;
    return g1_mpy->empty() ||
           bg::covered_by(*down_cast<Cartesian_multipolygon *>(g1_mpy.get()),
                          *g2);
  }
  if (eval(down_cast<Cartesian_multipolygon *>(g1_mpy.get()), g2)) {
    for (auto &pt : *down_cast<Cartesian_multipoint *>(g1_mpt.get()))
      if (!bg::covered_by(pt, *g2)) return false;
    return g1_mls->empty() ||
           bg::covered_by(*down_cast<Cartesian_multilinestring *>(g1_mls.get()),
                          *g2);
  }
  return false;
}

bool Within::eval(const Cartesian_geometrycollection *g1,
                  const Cartesian_geometrycollection *g2) const {
  // At least one element of g1 has to be within g2. The rest have to be covered
  // by g2.
  std::unique_ptr<Multipoint> g1_mpt;
  std::unique_ptr<Multilinestring> g1_mls;
  std::unique_ptr<Multipolygon> g1_mpy;
  split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
           &g1_mpy);
  gc_union(m_semi_major, m_semi_minor, &g1_mpt, &g1_mls, &g1_mpy);

  std::unique_ptr<Multipoint> g2_mpt;
  std::unique_ptr<Multilinestring> g2_mls;
  std::unique_ptr<Multipolygon> g2_mpy;
  split_gc(down_cast<const Geometrycollection *>(g2), &g2_mpt, &g2_mls,
           &g2_mpy);
  gc_union(m_semi_major, m_semi_minor, &g2_mpt, &g2_mls, &g2_mpy);

  // Check that no part of g1 is in the exterior of g2.
  Difference difference(m_semi_major, m_semi_minor);
  std::unique_ptr<Cartesian_multipoint> g1_mpt_diff_g2(
      new Cartesian_multipoint());
  g1_mpt_diff_g2.reset(down_cast<Cartesian_multipoint *>(
      difference(down_cast<Cartesian_multipoint *>(g1_mpt.get()),
                 down_cast<Cartesian_multipoint *>(g2_mpt.get()))));
  g1_mpt_diff_g2.reset(down_cast<Cartesian_multipoint *>(
      difference(g1_mpt_diff_g2.get(),
                 down_cast<Cartesian_multilinestring *>(g2_mls.get()))));
  g1_mpt_diff_g2.reset(down_cast<Cartesian_multipoint *>(
      difference(g1_mpt_diff_g2.get(),
                 down_cast<Cartesian_multipolygon *>(g2_mpy.get()))));
  if (!g1_mpt_diff_g2->empty()) return false;

  std::unique_ptr<Cartesian_multilinestring> g1_mls_diff_g2(
      new Cartesian_multilinestring());
  g1_mls_diff_g2.reset(down_cast<Cartesian_multilinestring *>(
      difference(down_cast<Cartesian_multilinestring *>(g1_mls.get()),
                 down_cast<Cartesian_multilinestring *>(g2_mls.get()))));
  g1_mls_diff_g2.reset(down_cast<Cartesian_multilinestring *>(
      difference(g1_mls_diff_g2.get(),
                 down_cast<Cartesian_multipolygon *>(g2_mpy.get()))));
  if (!g1_mls_diff_g2->empty()) return false;

  std::unique_ptr<Cartesian_multipolygon> g1_mpy_diff_g2(
      new Cartesian_multipolygon());
  g1_mpy_diff_g2.reset(down_cast<Cartesian_multipolygon *>(
      difference(down_cast<Cartesian_multipolygon *>(g1_mpy.get()),
                 down_cast<Cartesian_multipolygon *>(g2_mpy.get()))));
  if (!g1_mpy_diff_g2->empty()) return false;

  // Check that the interiors of g1 and g2 have at least one point in common.
  boost::geometry::de9im::mask mask("T********");
  return eval(down_cast<Cartesian_multipoint *>(g1_mpt.get()), g2) ||
         bg::relate(*down_cast<Cartesian_multilinestring *>(g1_mls.get()),
                    *down_cast<Cartesian_multilinestring *>(g2_mls.get()),
                    mask) ||
         bg::relate(*down_cast<Cartesian_multilinestring *>(g1_mls.get()),
                    *down_cast<Cartesian_multipolygon *>(g2_mpy.get()), mask) ||
         bg::relate(*down_cast<Cartesian_multipolygon *>(g1_mpy.get()),
                    *down_cast<Cartesian_multipolygon *>(g2_mpy.get()), mask);
}

bool Within::eval(const Cartesian_geometrycollection *g1,
                  const Cartesian_multipoint *g2) const {
  // g1 is within g2 if g1 contains only points and those points are within g2.
  std::unique_ptr<Multipoint> g1_mpt;
  std::unique_ptr<Multilinestring> g1_mls;
  std::unique_ptr<Multipolygon> g1_mpy;
  split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
           &g1_mpy);
  gc_union(m_semi_major, m_semi_minor, &g1_mpt, &g1_mls, &g1_mpy);
  return g1_mls->empty() && g1_mpy->empty() &&
         eval(down_cast<Cartesian_multipoint *>(g1_mpt.get()), g2);
}

bool Within::eval(const Cartesian_geometrycollection *g1,
                  const Cartesian_multilinestring *g2) const {
  // g1 is within g2 if g1 contains only points and linestrings. One of the
  // elements of g1 must be within g2, the rest must be covered by g2.
  std::unique_ptr<Multipoint> g1_mpt;
  std::unique_ptr<Multilinestring> g1_mls;
  std::unique_ptr<Multipolygon> g1_mpy;
  split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
           &g1_mpy);
  gc_union(m_semi_major, m_semi_minor, &g1_mpt, &g1_mls, &g1_mpy);

  if (!g1_mpy->empty()) return false;

  if (eval(down_cast<Cartesian_multipoint *>(g1_mpt.get()), g2))
    return g1_mls->empty() ||
           bg::covered_by(*down_cast<Cartesian_multilinestring *>(g1_mls.get()),
                          *g2);
  if (eval(down_cast<Cartesian_multilinestring *>(g1_mls.get()), g2)) {
    for (auto &pt : *down_cast<Cartesian_multipoint *>(g1_mpt.get()))
      if (!bg::covered_by(pt, *g2)) return false;
    return true;
  }
  return false;
}

bool Within::eval(const Cartesian_geometrycollection *g1,
                  const Cartesian_multipolygon *g2) const {
  // At least one element of g1 has to be within g2. The rest have to be covered
  // by g2.
  std::unique_ptr<Multipoint> g1_mpt;
  std::unique_ptr<Multilinestring> g1_mls;
  std::unique_ptr<Multipolygon> g1_mpy;
  split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
           &g1_mpy);
  gc_union(m_semi_major, m_semi_minor, &g1_mpt, &g1_mls, &g1_mpy);

  if (eval(down_cast<Cartesian_multipoint *>(g1_mpt.get()), g2)) {
    return (g1_mls->empty() ||
            bg::covered_by(
                *down_cast<Cartesian_multilinestring *>(g1_mls.get()), *g2)) &&
           (g1_mpy->empty() ||
            bg::covered_by(*down_cast<Cartesian_multipolygon *>(g1_mpy.get()),
                           *g2));
  }
  if (eval(down_cast<Cartesian_multilinestring *>(g1_mls.get()), g2)) {
    for (auto &pt : *down_cast<Cartesian_multipoint *>(g1_mpt.get()))
      if (!bg::covered_by(pt, *g2)) return false;
    return g1_mpy->empty() ||
           bg::covered_by(*down_cast<Cartesian_multipolygon *>(g1_mpy.get()),
                          *g2);
  }
  if (eval(down_cast<Cartesian_multipolygon *>(g1_mpy.get()), g2)) {
    for (auto &pt : *down_cast<Cartesian_multipoint *>(g1_mpt.get()))
      if (!bg::covered_by(pt, *g2)) return false;
    return g1_mls->empty() ||
           bg::covered_by(*down_cast<Cartesian_multilinestring *>(g1_mls.get()),
                          *g2);
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////

// within(Cartesian_multipoint, *)

bool Within::eval(const Cartesian_multipoint *g1,
                  const Cartesian_point *g2) const {
  Equals equals(m_semi_major, m_semi_minor);
  return equals(g1, g2);
}

bool Within::eval(const Cartesian_multipoint *g1,
                  const Cartesian_linestring *g2) const {
  // At least one point in g1 must be within g2. The rest has to intersect g2.
  bool within = false;
  bool intersects = false;
  for (auto &pt : *g1) {
    if (!within) {
      within = bg::within(pt, *g2);
      if (!within)
        intersects = bg::intersects(pt, *g2);
      else
        intersects = true;
    } else {
      intersects = bg::intersects(pt, *g2);
    }
    if (!intersects) break;
  }
  return (within && intersects);
}

bool Within::eval(const Cartesian_multipoint *g1,
                  const Cartesian_polygon *g2) const {
  // At least one point in g1 must be within g2. The rest has to intersect g2.
  bool within = false;
  bool intersects = false;
  for (auto &pt : *g1) {
    if (!within) {
      within = bg::within(pt, *g2);
      if (!within)
        intersects = bg::intersects(pt, *g2);
      else
        intersects = true;
    } else {
      intersects = bg::intersects(pt, *g2);
    }
    if (!intersects) break;
  }
  return (within && intersects);
}

bool Within::eval(const Cartesian_multipoint *g1,
                  const Cartesian_geometrycollection *g2) const {
  // At least one point in g1 must be within g2. The rest has to intersect g2.
  Intersects intersects_func(m_semi_major, m_semi_minor);
  bool within = false;
  bool intersects = false;
  for (auto &pt : *g1) {
    if (!within) {
      within = eval(&pt, g2);
      if (!within)
        intersects = intersects_func(&pt, g2);
      else
        intersects = true;
    } else {
      intersects = intersects_func(&pt, g2);
    }
    if (!intersects) break;
  }
  return (within && intersects);
}

bool Within::eval(const Cartesian_multipoint *g1,
                  const Cartesian_multipoint *g2) const {
  for (auto &pt : *g1)
    if (!bg::within(pt, *g2)) return false;
  return true;
}

bool Within::eval(const Cartesian_multipoint *g1,
                  const Cartesian_multilinestring *g2) const {
  // At least one point in g1 must be within g2. The rest has to intersect g2.
  bool within = false;
  bool intersects = false;
  for (auto &pt : *g1) {
    if (!within) {
      within = bg::within(pt, *g2);
      if (!within)
        intersects = bg::intersects(pt, *g2);
      else
        intersects = true;
    } else {
      intersects = bg::intersects(pt, *g2);
    }
    if (!intersects) break;
  }
  return (within && intersects);
}

bool Within::eval(const Cartesian_multipoint *g1,
                  const Cartesian_multipolygon *g2) const {
  // At least one point in g1 must be within g2. The rest has to intersect g2.
  bool within = false;
  bool intersects = false;
  for (auto &pt : *g1) {
    if (!within) {
      within = bg::within(pt, *g2);
      if (!within)
        intersects = bg::intersects(pt, *g2);
      else
        intersects = true;
    } else {
      intersects = bg::intersects(pt, *g2);
    }
    if (!intersects) break;
  }
  return (within && intersects);
}

//////////////////////////////////////////////////////////////////////////////

// within(Cartesian_multilinestring, *)

bool Within::eval(const Cartesian_multilinestring *,
                  const Cartesian_point *) const {
  // A multilinestring can never be within a point.
  return false;
}

bool Within::eval(const Cartesian_multilinestring *g1,
                  const Cartesian_linestring *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_multilinestring *g1,
                  const Cartesian_polygon *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_multilinestring *g1,
                  const Cartesian_geometrycollection *g2) const {
  // For g1 to be within g2, no point of g1 may be in the exterior of g2 and at
  // least one point of the interior of g1 has to be within the interior of g2.

  std::unique_ptr<Multipoint> g2_mpt;
  std::unique_ptr<Multilinestring> g2_mls;
  std::unique_ptr<Multipolygon> g2_mpy;
  split_gc(down_cast<const Geometrycollection *>(g2), &g2_mpt, &g2_mls,
           &g2_mpy);
  gc_union(m_semi_major, m_semi_minor, &g2_mpt, &g2_mls, &g2_mpy);

  Difference difference(m_semi_major, m_semi_minor);
  std::unique_ptr<Cartesian_multilinestring> g1_diff_g2(
      new Cartesian_multilinestring());
  g1_diff_g2.reset(down_cast<Cartesian_multilinestring *>(
      difference(g1, down_cast<Cartesian_multilinestring *>(g2_mls.get()))));
  g1_diff_g2.reset(down_cast<Cartesian_multilinestring *>(difference(
      g1_diff_g2.get(), down_cast<Cartesian_multipolygon *>(g2_mpy.get()))));

  boost::geometry::de9im::mask mask("T********");
  return g1_diff_g2->empty() &&
         (bg::relate(*g1, *down_cast<Cartesian_multilinestring *>(g2_mls.get()),
                     mask) ||
          bg::relate(*g1, *down_cast<Cartesian_multipolygon *>(g2_mpy.get()),
                     mask));
}

bool Within::eval(const Cartesian_multilinestring *,
                  const Cartesian_multipoint *) const {
  // A multilinestring can never be within a multipoint.
  return false;
}

bool Within::eval(const Cartesian_multilinestring *g1,
                  const Cartesian_multilinestring *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_multilinestring *g1,
                  const Cartesian_multipolygon *g2) const {
  return bg::within(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// within(Cartesian_multipolygon, *)

bool Within::eval(const Cartesian_multipolygon *,
                  const Cartesian_point *) const {
  // A multipolygon can never be within a point.
  return false;
}

bool Within::eval(const Cartesian_multipolygon *,
                  const Cartesian_linestring *) const {
  // A multipolygon can never be within a linestring.
  return false;
}

bool Within::eval(const Cartesian_multipolygon *g1,
                  const Cartesian_polygon *g2) const {
  return bg::within(*g1, *g2);
}

bool Within::eval(const Cartesian_multipolygon *g1,
                  const Cartesian_geometrycollection *g2) const {
  // A multipolygon may not be within the points and linestrings of g2, so the
  // only way a multipolygon is within a geometrycollectin, is if it's within
  // the union multipolygon of the geometrycollection.
  std::unique_ptr<Multipoint> g2_mpt;
  std::unique_ptr<Multilinestring> g2_mls;
  std::unique_ptr<Multipolygon> g2_mpy;
  split_gc(down_cast<const Geometrycollection *>(g2), &g2_mpt, &g2_mls,
           &g2_mpy);
  gc_union(m_semi_major, m_semi_minor, &g2_mpt, &g2_mls, &g2_mpy);
  return eval(g1, down_cast<Cartesian_multipolygon *>(g2_mpy.get()));
}

bool Within::eval(const Cartesian_multipolygon *,
                  const Cartesian_multipoint *) const {
  // A multipolygon can never be within a multipoint.
  return false;
}

bool Within::eval(const Cartesian_multipolygon *,
                  const Cartesian_multilinestring *) const {
  // A multipolygon can never be within a multilinestring.
  return false;
}

bool Within::eval(const Cartesian_multipolygon *g1,
                  const Cartesian_multipolygon *g2) const {
  return bg::within(*g1, *g2);
}

//////////////////////////////////////////////////////////////////////////////

// within(Geographic_point, *)

bool Within::eval(const Geographic_point *g1,
                  const Geographic_point *g2) const {
  // Default strategy is OK. P/P computations do not depend on shape of
  // ellipsoid.
  return bg::within(*g1, *g2);
}

bool Within::eval(const Geographic_point *g1,
                  const Geographic_linestring *g2) const {
  return bg::within(*g1, *g2, m_geographic_pl_pa_strategy);
}

bool Within::eval(const Geographic_point *g1,
                  const Geographic_polygon *g2) const {
  return bg::within(*g1, *g2, m_geographic_pl_pa_strategy);
}

bool Within::eval(const Geographic_point *g1,
                  const Geographic_geometrycollection *g2) const {
  // g1 must be within one of the elements of g2.
  for (auto g : *g2)
    if ((*this)(g1, g)) return true;
  return false;
}

bool Within::eval(const Geographic_point *g1,
                  const Geographic_multipoint *g2) const {
  // Default strategy is OK. P/P computations do not depend on shape of
  // ellipsoid.
  return bg::within(*g1, *g2);
}

bool Within::eval(const Geographic_point *g1,
                  const Geographic_multilinestring *g2) const {
  return bg::within(*g1, *g2, m_geographic_pl_pa_strategy);
}

bool Within::eval(const Geographic_point *g1,
                  const Geographic_multipolygon *g2) const {
  return bg::within(*g1, *g2, m_geographic_pl_pa_strategy);
}

//////////////////////////////////////////////////////////////////////////////

// within(Geographic_linestring, *)

bool Within::eval(const Geographic_linestring *,
                  const Geographic_point *) const {
  // A linestring can never be within a point.
  return false;
}

bool Within::eval(const Geographic_linestring *g1,
                  const Geographic_linestring *g2) const {
  return bg::within(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Within::eval(const Geographic_linestring *g1,
                  const Geographic_polygon *g2) const {
  return bg::within(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Within::eval(const Geographic_linestring *g1,
                  const Geographic_geometrycollection *g2) const {
  // For g1 to be within g2, no point of g1 may be in the exterior of g2 and at
  // least one point of the interior of g1 has to be within the interior of g2.

  std::unique_ptr<Multipoint> g2_mpt;
  std::unique_ptr<Multilinestring> g2_mls;
  std::unique_ptr<Multipolygon> g2_mpy;
  split_gc(down_cast<const Geometrycollection *>(g2), &g2_mpt, &g2_mls,
           &g2_mpy);
  gc_union(m_semi_major, m_semi_minor, &g2_mpt, &g2_mls, &g2_mpy);

  Difference difference(m_semi_major, m_semi_minor);
  std::unique_ptr<Multilinestring> g1_diff_g2(new Geographic_multilinestring());
  g1_diff_g2.reset(down_cast<Geographic_multilinestring *>(
      difference(g1, down_cast<Geographic_multilinestring *>(g2_mls.get()))));
  g1_diff_g2.reset(down_cast<Geographic_multilinestring *>(
      difference(down_cast<Geographic_multilinestring *>(g1_diff_g2.get()),
                 down_cast<Geographic_multipolygon *>(g2_mpy.get()))));

  boost::geometry::de9im::mask mask("T********");
  return g1_diff_g2->empty() &&
         (bg::relate(*g1,
                     *down_cast<Geographic_multilinestring *>(g2_mls.get()),
                     mask, m_geographic_ll_la_aa_strategy) ||
          bg::relate(*g1, *down_cast<Geographic_multipolygon *>(g2_mpy.get()),
                     mask, m_geographic_ll_la_aa_strategy));
}

bool Within::eval(const Geographic_linestring *,
                  const Geographic_multipoint *) const {
  // A linestring can never be within a multipoint.
  return false;
}

bool Within::eval(const Geographic_linestring *g1,
                  const Geographic_multilinestring *g2) const {
  return bg::within(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Within::eval(const Geographic_linestring *g1,
                  const Geographic_multipolygon *g2) const {
  return bg::within(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

//////////////////////////////////////////////////////////////////////////////

// within(Geographic_polygon, *)

bool Within::eval(const Geographic_polygon *, const Geographic_point *) const {
  // A polygon can never be within a point.
  return false;
}

bool Within::eval(const Geographic_polygon *,
                  const Geographic_linestring *) const {
  // A polygon can never be within a linestring.
  return false;
}

bool Within::eval(const Geographic_polygon *g1,
                  const Geographic_polygon *g2) const {
  return bg::within(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Within::eval(const Geographic_polygon *g1,
                  const Geographic_geometrycollection *g2) const {
  // Polygons can only be within other polygons or multipolygons, so we can
  // ignore points and linestrings. g1 is within g2 if g1 is within the union
  // multipolygon of g2.
  std::unique_ptr<Multipoint> g2_mpt;
  std::unique_ptr<Multilinestring> g2_mls;
  std::unique_ptr<Multipolygon> g2_mpy;
  split_gc(down_cast<const Geometrycollection *>(g2), &g2_mpt, &g2_mls,
           &g2_mpy);
  gc_union(m_semi_major, m_semi_minor, &g2_mpt, &g2_mls, &g2_mpy);
  return eval(g1, down_cast<Geographic_multipolygon *>(g2_mpy.get()));
}

bool Within::eval(const Geographic_polygon *,
                  const Geographic_multipoint *) const {
  // A polygon can never be within a multipoint.
  return false;
}

bool Within::eval(const Geographic_polygon *,
                  const Geographic_multilinestring *) const {
  // A polygon can never be within a multilinestring.
  return false;
}

bool Within::eval(const Geographic_polygon *g1,
                  const Geographic_multipolygon *g2) const {
  return bg::within(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

//////////////////////////////////////////////////////////////////////////////

// within(Geographic_geometrycollection, *)

bool Within::eval(const Geographic_geometrycollection *g1,
                  const Geographic_point *g2) const {
  Equals equals(m_semi_major, m_semi_minor);
  return equals(g1, g2);
}

bool Within::eval(const Geographic_geometrycollection *g1,
                  const Geographic_linestring *g2) const {
  // g1 is within g2 if g1 contains only points and linestrings. One of the
  // elements of g1 must be within g2, the rest must be covered by g2.
  std::unique_ptr<Multipoint> g1_mpt;
  std::unique_ptr<Multilinestring> g1_mls;
  std::unique_ptr<Multipolygon> g1_mpy;
  split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
           &g1_mpy);
  gc_union(m_semi_major, m_semi_minor, &g1_mpt, &g1_mls, &g1_mpy);

  if (!g1_mpy->empty()) return false;

  if (eval(down_cast<Geographic_multipoint *>(g1_mpt.get()), g2))
    return g1_mls->empty() ||
           bg::covered_by(
               *down_cast<Geographic_multilinestring *>(g1_mls.get()), *g2,
               m_geographic_ll_la_aa_strategy);
  if (eval(down_cast<Geographic_multilinestring *>(g1_mls.get()), g2)) {
    for (auto &pt : *down_cast<Geographic_multipoint *>(g1_mpt.get()))
      if (!bg::covered_by(pt, *g2, m_geographic_pl_pa_strategy)) return false;
    return true;
  }
  return false;
}

bool Within::eval(const Geographic_geometrycollection *g1,
                  const Geographic_polygon *g2) const {
  // At least one element of g1 has to be within g2. The rest have to be covered
  // by g2.
  std::unique_ptr<Multipoint> g1_mpt;
  std::unique_ptr<Multilinestring> g1_mls;
  std::unique_ptr<Multipolygon> g1_mpy;
  split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
           &g1_mpy);
  gc_union(m_semi_major, m_semi_minor, &g1_mpt, &g1_mls, &g1_mpy);

  if (eval(down_cast<Geographic_multipoint *>(g1_mpt.get()), g2)) {
    return (g1_mls->empty() ||
            bg::covered_by(
                *down_cast<Geographic_multilinestring *>(g1_mls.get()), *g2,
                m_geographic_ll_la_aa_strategy)) &&
           (g1_mpy->empty() ||
            bg::covered_by(*down_cast<Geographic_multipolygon *>(g1_mpy.get()),
                           *g2, m_geographic_ll_la_aa_strategy));
  }
  if (eval(down_cast<Geographic_multilinestring *>(g1_mls.get()), g2)) {
    for (auto &pt : *down_cast<Geographic_multipoint *>(g1_mpt.get()))
      if (!bg::covered_by(pt, *g2, m_geographic_pl_pa_strategy)) return false;
    return g1_mpy->empty() ||
           bg::covered_by(*down_cast<Geographic_multipolygon *>(g1_mpy.get()),
                          *g2, m_geographic_ll_la_aa_strategy);
  }
  if (eval(down_cast<Geographic_multipolygon *>(g1_mpy.get()), g2)) {
    for (auto &pt : *down_cast<Geographic_multipoint *>(g1_mpt.get()))
      if (!bg::covered_by(pt, *g2, m_geographic_pl_pa_strategy)) return false;
    return g1_mls->empty() ||
           bg::covered_by(
               *down_cast<Geographic_multilinestring *>(g1_mls.get()), *g2,
               m_geographic_ll_la_aa_strategy);
  }
  return false;
}

bool Within::eval(const Geographic_geometrycollection *g1,
                  const Geographic_geometrycollection *g2) const {
  // At least one element of g1 has to be within g2. The rest have to be covered
  // by g2.
  std::unique_ptr<Multipoint> g1_mpt;
  std::unique_ptr<Multilinestring> g1_mls;
  std::unique_ptr<Multipolygon> g1_mpy;
  split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
           &g1_mpy);
  gc_union(m_semi_major, m_semi_minor, &g1_mpt, &g1_mls, &g1_mpy);

  std::unique_ptr<Multipoint> g2_mpt;
  std::unique_ptr<Multilinestring> g2_mls;
  std::unique_ptr<Multipolygon> g2_mpy;
  split_gc(down_cast<const Geometrycollection *>(g2), &g2_mpt, &g2_mls,
           &g2_mpy);
  gc_union(m_semi_major, m_semi_minor, &g2_mpt, &g2_mls, &g2_mpy);

  // Check that no part of g1 is in the exterior of g2.
  Difference difference(m_semi_major, m_semi_minor);
  std::unique_ptr<Geographic_multipoint> g1_mpt_diff_g2(
      new Geographic_multipoint());
  g1_mpt_diff_g2.reset(down_cast<Geographic_multipoint *>(
      difference(down_cast<Geographic_multipoint *>(g1_mpt.get()),
                 down_cast<Geographic_multipoint *>(g2_mpt.get()))));
  g1_mpt_diff_g2.reset(down_cast<Geographic_multipoint *>(
      difference(g1_mpt_diff_g2.get(),
                 down_cast<Geographic_multilinestring *>(g2_mls.get()))));
  g1_mpt_diff_g2.reset(down_cast<Geographic_multipoint *>(
      difference(g1_mpt_diff_g2.get(),
                 down_cast<Geographic_multipolygon *>(g2_mpy.get()))));
  if (!g1_mpt_diff_g2->empty()) return false;

  std::unique_ptr<Geographic_multilinestring> g1_mls_diff_g2(
      new Geographic_multilinestring());
  g1_mls_diff_g2.reset(down_cast<Geographic_multilinestring *>(
      difference(down_cast<Geographic_multilinestring *>(g1_mls.get()),
                 down_cast<Geographic_multilinestring *>(g2_mls.get()))));
  g1_mls_diff_g2.reset(down_cast<Geographic_multilinestring *>(
      difference(g1_mls_diff_g2.get(),
                 down_cast<Geographic_multipolygon *>(g2_mpy.get()))));
  if (!g1_mls_diff_g2->empty()) return false;

  std::unique_ptr<Geographic_multipolygon> g1_mpy_diff_g2(
      new Geographic_multipolygon());
  g1_mpy_diff_g2.reset(down_cast<Geographic_multipolygon *>(
      difference(down_cast<Geographic_multipolygon *>(g1_mpy.get()),
                 down_cast<Geographic_multipolygon *>(g2_mpy.get()))));
  if (!g1_mpy_diff_g2->empty()) return false;

  // Check that the interiors of g1 and g2 have at least one point in common.
  boost::geometry::de9im::mask mask("T********");
  return eval(down_cast<Geographic_multipoint *>(g1_mpt.get()), g2) ||
         bg::relate(*down_cast<Geographic_multilinestring *>(g1_mls.get()),
                    *down_cast<Geographic_multilinestring *>(g2_mls.get()),
                    mask, m_geographic_ll_la_aa_strategy) ||
         bg::relate(*down_cast<Geographic_multilinestring *>(g1_mls.get()),
                    *down_cast<Geographic_multipolygon *>(g2_mpy.get()), mask,
                    m_geographic_ll_la_aa_strategy) ||
         bg::relate(*down_cast<Geographic_multipolygon *>(g1_mpy.get()),
                    *down_cast<Geographic_multipolygon *>(g2_mpy.get()), mask,
                    m_geographic_ll_la_aa_strategy);
}

bool Within::eval(const Geographic_geometrycollection *g1,
                  const Geographic_multipoint *g2) const {
  // g1 is within g2 if g1 contains only points and those points are within g2.
  std::unique_ptr<Multipoint> g1_mpt;
  std::unique_ptr<Multilinestring> g1_mls;
  std::unique_ptr<Multipolygon> g1_mpy;
  split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
           &g1_mpy);
  gc_union(m_semi_major, m_semi_minor, &g1_mpt, &g1_mls, &g1_mpy);
  return g1_mls->empty() && g1_mpy->empty() &&
         eval(down_cast<Geographic_multipoint *>(g1_mpt.get()), g2);
}

bool Within::eval(const Geographic_geometrycollection *g1,
                  const Geographic_multilinestring *g2) const {
  // g1 is within g2 if g1 contains only points and linestrings. One of the
  // elements of g1 must be within g2, the rest must be covered by g2.
  std::unique_ptr<Multipoint> g1_mpt;
  std::unique_ptr<Multilinestring> g1_mls;
  std::unique_ptr<Multipolygon> g1_mpy;
  split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
           &g1_mpy);
  gc_union(m_semi_major, m_semi_minor, &g1_mpt, &g1_mls, &g1_mpy);

  if (!g1_mpy->empty()) return false;

  if (eval(down_cast<Geographic_multipoint *>(g1_mpt.get()), g2))
    return g1_mls->empty() ||
           bg::covered_by(
               *down_cast<Geographic_multilinestring *>(g1_mls.get()), *g2,
               m_geographic_ll_la_aa_strategy);
  if (eval(down_cast<Geographic_multilinestring *>(g1_mls.get()), g2)) {
    for (auto &pt : *down_cast<Geographic_multipoint *>(g1_mpt.get()))
      if (!bg::covered_by(pt, *g2, m_geographic_pl_pa_strategy)) return false;
    return true;
  }
  return false;
}

bool Within::eval(const Geographic_geometrycollection *g1,
                  const Geographic_multipolygon *g2) const {
  // At least one element of g1 has to be within g2. The rest have to be covered
  // by g2.
  std::unique_ptr<Multipoint> g1_mpt;
  std::unique_ptr<Multilinestring> g1_mls;
  std::unique_ptr<Multipolygon> g1_mpy;
  split_gc(down_cast<const Geometrycollection *>(g1), &g1_mpt, &g1_mls,
           &g1_mpy);
  gc_union(m_semi_major, m_semi_minor, &g1_mpt, &g1_mls, &g1_mpy);

  if (eval(down_cast<Geographic_multipoint *>(g1_mpt.get()), g2)) {
    return (g1_mls->empty() ||
            bg::covered_by(
                *down_cast<Geographic_multilinestring *>(g1_mls.get()), *g2,
                m_geographic_ll_la_aa_strategy)) &&
           (g1_mpy->empty() ||
            bg::covered_by(*down_cast<Geographic_multipolygon *>(g1_mpy.get()),
                           *g2, m_geographic_ll_la_aa_strategy));
  }
  if (eval(down_cast<Geographic_multilinestring *>(g1_mls.get()), g2)) {
    for (auto &pt : *down_cast<Geographic_multipoint *>(g1_mpt.get()))
      if (!bg::covered_by(pt, *g2, m_geographic_pl_pa_strategy)) return false;
    return g1_mpy->empty() ||
           bg::covered_by(*down_cast<Geographic_multipolygon *>(g1_mpy.get()),
                          *g2, m_geographic_ll_la_aa_strategy);
  }
  if (eval(down_cast<Geographic_multipolygon *>(g1_mpy.get()), g2)) {
    for (auto &pt : *down_cast<Geographic_multipoint *>(g1_mpt.get()))
      if (!bg::covered_by(pt, *g2, m_geographic_pl_pa_strategy)) return false;
    return g1_mls->empty() ||
           bg::covered_by(
               *down_cast<Geographic_multilinestring *>(g1_mls.get()), *g2,
               m_geographic_ll_la_aa_strategy);
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////

// within(Geographic_multipoint, *)

bool Within::eval(const Geographic_multipoint *g1,
                  const Geographic_point *g2) const {
  Equals equals(m_semi_major, m_semi_minor);
  return equals(g1, g2);
}

bool Within::eval(const Geographic_multipoint *g1,
                  const Geographic_linestring *g2) const {
  // At least one point in g1 must be within g2. The rest has to intersect g2.
  bool within = false;
  bool intersects = false;
  for (auto &pt : *g1) {
    if (!within) {
      within = bg::within(pt, *g2, m_geographic_pl_pa_strategy);
      if (!within)
        intersects = bg::intersects(pt, *g2, m_geographic_pl_pa_strategy);
      else
        intersects = true;
    } else {
      intersects = bg::intersects(pt, *g2, m_geographic_pl_pa_strategy);
    }
    if (!intersects) break;
  }
  return (within && intersects);
}

bool Within::eval(const Geographic_multipoint *g1,
                  const Geographic_polygon *g2) const {
  // At least one point in g1 must be within g2. The rest has to intersect g2.
  bool within = false;
  bool intersects = false;
  for (auto &pt : *g1) {
    if (!within) {
      within = bg::within(pt, *g2, m_geographic_pl_pa_strategy);
      if (!within)
        intersects = bg::intersects(pt, *g2, m_geographic_pl_pa_strategy);
      else
        intersects = true;
    } else {
      intersects = bg::intersects(pt, *g2, m_geographic_pl_pa_strategy);
    }
    if (!intersects) break;
  }
  return (within && intersects);
}

bool Within::eval(const Geographic_multipoint *g1,
                  const Geographic_geometrycollection *g2) const {
  // At least one point in g1 must be within g2. The rest has to intersect g2.
  Intersects intersects_func(m_semi_major, m_semi_minor);
  bool within = false;
  bool intersects = false;
  for (auto &pt : *g1) {
    if (!within) {
      within = eval(&pt, g2);
      if (!within)
        intersects = intersects_func(&pt, g2);
      else
        intersects = true;
    } else {
      intersects = intersects_func(&pt, g2);
    }
    if (!intersects) break;
  }
  return (within && intersects);
}

bool Within::eval(const Geographic_multipoint *g1,
                  const Geographic_multipoint *g2) const {
  // Default strategy is OK. P/P computations do not depend on shape of
  // ellipsoid.
  for (auto &pt : *g1)
    if (!bg::within(pt, *g2)) return false;
  return true;
}

bool Within::eval(const Geographic_multipoint *g1,
                  const Geographic_multilinestring *g2) const {
  // At least one point in g1 must be within g2. The rest has to intersect g2.
  bool within = false;
  bool intersects = false;
  for (auto &pt : *g1) {
    if (!within) {
      within = bg::within(pt, *g2, m_geographic_pl_pa_strategy);
      if (!within)
        intersects = bg::intersects(pt, *g2, m_geographic_pl_pa_strategy);
      else
        intersects = true;
    } else {
      intersects = bg::intersects(pt, *g2, m_geographic_pl_pa_strategy);
    }
    if (!intersects) break;
  }
  return (within && intersects);
}

bool Within::eval(const Geographic_multipoint *g1,
                  const Geographic_multipolygon *g2) const {
  // At least one point in g1 must be within g2. The rest has to intersect g2.
  bool within = false;
  bool intersects = false;
  for (auto &pt : *g1) {
    if (!within) {
      within = bg::within(pt, *g2, m_geographic_pl_pa_strategy);
      if (!within)
        intersects = bg::intersects(pt, *g2, m_geographic_pl_pa_strategy);
      else
        intersects = true;
    } else {
      intersects = bg::intersects(pt, *g2, m_geographic_pl_pa_strategy);
    }
    if (!intersects) break;
  }
  return (within && intersects);
}

//////////////////////////////////////////////////////////////////////////////

// within(Geographic_multilinestring, *)

bool Within::eval(const Geographic_multilinestring *,
                  const Geographic_point *) const {
  // A multilinestring can never be within a point.
  return false;
}

bool Within::eval(const Geographic_multilinestring *g1,
                  const Geographic_linestring *g2) const {
  return bg::within(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Within::eval(const Geographic_multilinestring *g1,
                  const Geographic_polygon *g2) const {
  return bg::within(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Within::eval(const Geographic_multilinestring *g1,
                  const Geographic_geometrycollection *g2) const {
  // For g1 to be within g2, no point of g1 may be in the exterior of g2 and at
  // least one point of the interior of g1 has to be within the interior of g2.

  std::unique_ptr<Multipoint> g2_mpt;
  std::unique_ptr<Multilinestring> g2_mls;
  std::unique_ptr<Multipolygon> g2_mpy;
  split_gc(down_cast<const Geometrycollection *>(g2), &g2_mpt, &g2_mls,
           &g2_mpy);
  gc_union(m_semi_major, m_semi_minor, &g2_mpt, &g2_mls, &g2_mpy);

  Difference difference(m_semi_major, m_semi_minor);
  std::unique_ptr<Geographic_multilinestring> g1_diff_g2(
      new Geographic_multilinestring());
  g1_diff_g2.reset(down_cast<Geographic_multilinestring *>(
      difference(g1, down_cast<Geographic_multilinestring *>(g2_mls.get()))));
  g1_diff_g2.reset(down_cast<Geographic_multilinestring *>(difference(
      g1_diff_g2.get(), down_cast<Geographic_multipolygon *>(g2_mpy.get()))));

  boost::geometry::de9im::mask mask("T********");
  return g1_diff_g2->empty() &&
         (bg::relate(*g1,
                     *down_cast<Geographic_multilinestring *>(g2_mls.get()),
                     mask, m_geographic_ll_la_aa_strategy) ||
          bg::relate(*g1, *down_cast<Geographic_multipolygon *>(g2_mpy.get()),
                     mask, m_geographic_ll_la_aa_strategy));
}

bool Within::eval(const Geographic_multilinestring *,
                  const Geographic_multipoint *) const {
  // A multilinestring can never be within a multipoint.
  return false;
}

bool Within::eval(const Geographic_multilinestring *g1,
                  const Geographic_multilinestring *g2) const {
  return bg::within(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Within::eval(const Geographic_multilinestring *g1,
                  const Geographic_multipolygon *g2) const {
  return bg::within(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

//////////////////////////////////////////////////////////////////////////////

// within(Geographic_multipolygon, *)

bool Within::eval(const Geographic_multipolygon *,
                  const Geographic_point *) const {
  // A multipolygon can never be within a point.
  return false;
}

bool Within::eval(const Geographic_multipolygon *,
                  const Geographic_linestring *) const {
  // A multipolygon can never be within a linestring.
  return false;
}

bool Within::eval(const Geographic_multipolygon *g1,
                  const Geographic_polygon *g2) const {
  return bg::within(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

bool Within::eval(const Geographic_multipolygon *g1,
                  const Geographic_geometrycollection *g2) const {
  // A multipolygon may not be within the points and linestrings of g2, so the
  // only way a multipolygon is within a geometrycollectin, is if it's within
  // the union multipolygon of the geometrycollection.
  std::unique_ptr<Multipoint> g2_mpt;
  std::unique_ptr<Multilinestring> g2_mls;
  std::unique_ptr<Multipolygon> g2_mpy;
  split_gc(down_cast<const Geometrycollection *>(g2), &g2_mpt, &g2_mls,
           &g2_mpy);
  gc_union(m_semi_major, m_semi_minor, &g2_mpt, &g2_mls, &g2_mpy);
  return eval(g1, down_cast<Geographic_multipolygon *>(g2_mpy.get()));
}

bool Within::eval(const Geographic_multipolygon *,
                  const Geographic_multipoint *) const {
  // A multipolygon can never be within a multipoint.
  return false;
}

bool Within::eval(const Geographic_multipolygon *,
                  const Geographic_multilinestring *) const {
  // A multipolygon can never be within a multilinestring.
  return false;
}

bool Within::eval(const Geographic_multipolygon *g1,
                  const Geographic_multipolygon *g2) const {
  return bg::within(*g1, *g2, m_geographic_ll_la_aa_strategy);
}

//////////////////////////////////////////////////////////////////////////////

// within(Box, Box)

bool Within::eval(const Cartesian_box *b1, const Cartesian_box *b2) const {
  if (mbrs_are_equal(*b1, *b2)) return true;

  // Work around bugs in BG for boxes that have zero height and/or width.
  if (mbr_is_point(*b1)) {
    Cartesian_point pt(b1->min_corner().x(), b1->min_corner().y());

    if (mbr_is_line(*b2)) {
      Cartesian_point b2_ls_start(b2->min_corner().x(), b2->min_corner().y());
      Cartesian_point b2_ls_end(b2->max_corner().x(), b2->max_corner().y());
      Cartesian_linestring b2_ls;
      b2_ls.push_back(b2_ls_start);
      b2_ls.push_back(b2_ls_end);

      return bg::within(pt, b2_ls);
    }

    return bg::within(pt, *b2);
  }

  if (mbr_is_line(*b1)) {
    Cartesian_point b1_ls_start(b1->min_corner().x(), b1->min_corner().y());
    Cartesian_point b1_ls_end(b1->max_corner().x(), b1->max_corner().y());
    Cartesian_linestring b1_ls;
    b1_ls.push_back(b1_ls_start);
    b1_ls.push_back(b1_ls_end);

    if (mbr_is_line(*b2)) {
      Cartesian_point b2_ls_start(b2->min_corner().x(), b2->min_corner().y());
      Cartesian_point b2_ls_end(b2->max_corner().x(), b2->max_corner().y());
      Cartesian_linestring b2_ls;
      b2_ls.push_back(b2_ls_start);
      b2_ls.push_back(b2_ls_end);

      return bg::within(b1_ls, b2_ls);
    }

    Cartesian_point b2_pt1(b2->min_corner().x(), b2->min_corner().y());
    Cartesian_point b2_pt2(b2->max_corner().x(), b2->min_corner().y());
    Cartesian_point b2_pt3(b2->max_corner().x(), b2->max_corner().y());
    Cartesian_point b2_pt4(b2->min_corner().x(), b2->max_corner().y());
    Cartesian_point b2_pt5(b2->min_corner().x(), b2->min_corner().y());
    Cartesian_linearring b2_lr;
    b2_lr.push_back(b2_pt1);
    b2_lr.push_back(b2_pt2);
    b2_lr.push_back(b2_pt3);
    b2_lr.push_back(b2_pt4);
    b2_lr.push_back(b2_pt5);
    Cartesian_polygon b2_py;
    b2_py.push_back(b2_lr);

    return bg::within(b1_ls, b2_py);
  }

  return bg::within(*b1, *b2);
}

bool Within::eval(const Geographic_box *b1, const Geographic_box *b2) const {
  if (mbrs_are_equal(*b1, *b2)) return true;

  // Work around bugs in BG for boxes that have zero height and/or width.
  if (mbr_is_point(*b1)) {
    Geographic_point pt(b1->min_corner().x(), b1->min_corner().y());

    if (mbr_is_line(*b2)) {
      Geographic_point b2_ls_start(b2->min_corner().x(), b2->min_corner().y());
      Geographic_point b2_ls_end(b2->max_corner().x(), b2->max_corner().y());
      Geographic_linestring b2_ls;
      b2_ls.push_back(b2_ls_start);
      b2_ls.push_back(b2_ls_end);

      return bg::within(pt, b2_ls);
    }

    return bg::within(pt, *b2);
  }

  if (mbr_is_line(*b1)) {
    Geographic_point b1_ls_start(b1->min_corner().x(), b1->min_corner().y());
    Geographic_point b1_ls_end(b1->max_corner().x(), b1->max_corner().y());
    Geographic_linestring b1_ls;
    b1_ls.push_back(b1_ls_start);
    b1_ls.push_back(b1_ls_end);

    if (mbr_is_line(*b2)) {
      Geographic_point b2_ls_start(b2->min_corner().x(), b2->min_corner().y());
      Geographic_point b2_ls_end(b2->max_corner().x(), b2->max_corner().y());
      Geographic_linestring b2_ls;
      b2_ls.push_back(b2_ls_start);
      b2_ls.push_back(b2_ls_end);

      return bg::within(b1_ls, b2_ls);
    }

    // If b1 is a line along the border of b2, it's not within b2.
    if (((b1_ls_start.x() == b1_ls_end.x()) &&
         (b1_ls_start.x() == b2->min_corner().x() ||
          b1_ls_start.x() == b2->max_corner().x())) ||
        ((b1_ls_start.y() == b1_ls_end.y()) &&
         (b1_ls_start.y() == b2->min_corner().y() ||
          b1_ls_start.y() == b2->max_corner().y())))
      return false;

    return bg::covered_by(b1_ls_start, *b2) && bg::covered_by(b1_ls_end, *b2);
  }

  return bg::within(*b1, *b2);
}

//////////////////////////////////////////////////////////////////////////////

bool within(const dd::Spatial_reference_system *srs, const Geometry *g1,
            const Geometry *g2, const char *func_name, bool *within,
            bool *null) noexcept {
  try {
    DBUG_ASSERT(g1->coordinate_system() == g2->coordinate_system());
    DBUG_ASSERT(srs == nullptr ||
                ((srs->is_cartesian() &&
                  g1->coordinate_system() == Coordinate_system::kCartesian) ||
                 (srs->is_geographic() &&
                  g1->coordinate_system() == Coordinate_system::kGeographic)));

    if ((*null = (g1->is_empty() || g2->is_empty()))) return false;

    Within within_func(srs ? srs->semi_major_axis() : 0.0,
                       srs ? srs->semi_minor_axis() : 0.0);
    *within = within_func(g1, g2);
  } catch (...) {
    handle_gis_exception(func_name);
    return true;
  }

  return false;
}

bool mbr_within(const dd::Spatial_reference_system *srs, const Geometry *g1,
                const Geometry *g2, const char *func_name, bool *within,
                bool *null) noexcept {
  try {
    DBUG_ASSERT(g1->coordinate_system() == g2->coordinate_system());
    DBUG_ASSERT(srs == nullptr ||
                ((srs->is_cartesian() &&
                  g1->coordinate_system() == Coordinate_system::kCartesian) ||
                 (srs->is_geographic() &&
                  g1->coordinate_system() == Coordinate_system::kGeographic)));

    if ((*null = (g1->is_empty() || g2->is_empty()))) return false;

    Within within_func(srs ? srs->semi_major_axis() : 0.0,
                       srs ? srs->semi_minor_axis() : 0.0);

    switch (g1->coordinate_system()) {
      case Coordinate_system::kCartesian: {
        Cartesian_box mbr1;
        box_envelope(g1, srs, &mbr1);
        Cartesian_box mbr2;
        box_envelope(g2, srs, &mbr2);

        *within = within_func(&mbr1, &mbr2);
        break;
      }
      case Coordinate_system::kGeographic: {
        Geographic_box mbr1;
        box_envelope(g1, srs, &mbr1);
        Geographic_box mbr2;
        box_envelope(g2, srs, &mbr2);

        *within = within_func(&mbr1, &mbr2);
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
