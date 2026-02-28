// Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.
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
/// This file implements utility functions for working with geometrycollections.

#include "sql/gis/gc_utils.h"

#include <boost/geometry.hpp>  // boost::geometry::difference

#include "my_dbug.h"  // DBUG_ASSERT
#include "sql/gis/difference_functor.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_cs.h"
#include "sql/gis/geometries_traits.h"
#include "sql/gis/union_functor.h"
#include "template_utils.h"  // down_cast

namespace bg = boost::geometry;

namespace gis {

template <typename Pt, typename Ls, typename Py, typename GC, typename MPt,
          typename MLs, typename MPy>
static void typed_split_gc(const GC *gc, MPt *mpt, MLs *mls, MPy *mpy) {
  DBUG_ASSERT(gc->coordinate_system() == mpt->coordinate_system() &&
              gc->coordinate_system() == mls->coordinate_system() &&
              gc->coordinate_system() == mpy->coordinate_system());

  for (const auto g : *gc) {
    switch (g->type()) {
      case Geometry_type::kPoint:
        mpt->push_back(*down_cast<Pt *>(g));
        break;
      case Geometry_type::kLinestring:
        mls->push_back(*down_cast<Ls *>(g));
        break;
      case Geometry_type::kPolygon:
        mpy->push_back(*down_cast<Py *>(g));
        break;
      case Geometry_type::kGeometrycollection:
        typed_split_gc<Pt, Ls, Py, GC, MPt, MLs, MPy>(down_cast<GC *>(g), mpt,
                                                      mls, mpy);
        break;
      case Geometry_type::kMultipoint: {
        const MPt *m = down_cast<const MPt *>(g);
        for (std::size_t i = 0; i < m->size(); i++)
          mpt->push_back(static_cast<const Pt &>((*m)[i]));
        break;
      }
      case Geometry_type::kMultilinestring: {
        const MLs *m = down_cast<const MLs *>(g);
        for (std::size_t i = 0; i < m->size(); i++)
          mls->push_back(static_cast<const Ls &>((*m)[i]));
        break;
      }
      case Geometry_type::kMultipolygon: {
        const MPy *m = down_cast<const MPy *>(g);
        for (std::size_t i = 0; i < m->size(); i++)
          mpy->push_back(static_cast<const Py &>((*m)[i]));
        break;
      }
      case Geometry_type::kGeometry:
        DBUG_ASSERT(false);
        break;
    }
  }
}

void split_gc(const Geometrycollection *gc, std::unique_ptr<Multipoint> *mpt,
              std::unique_ptr<Multilinestring> *mls,
              std::unique_ptr<Multipolygon> *mpy) {
  switch (gc->coordinate_system()) {
    case Coordinate_system::kCartesian:
      mpt->reset(new Cartesian_multipoint());
      mls->reset(new Cartesian_multilinestring());
      mpy->reset(new Cartesian_multipolygon());
      typed_split_gc<Cartesian_point, Cartesian_linestring, Cartesian_polygon,
                     Cartesian_geometrycollection, Cartesian_multipoint,
                     Cartesian_multilinestring, Cartesian_multipolygon>(
          down_cast<const Cartesian_geometrycollection *>(gc),
          down_cast<Cartesian_multipoint *>(mpt->get()),
          down_cast<Cartesian_multilinestring *>(mls->get()),
          down_cast<Cartesian_multipolygon *>(mpy->get()));
      break;
    case Coordinate_system::kGeographic:
      mpt->reset(new Geographic_multipoint());
      mls->reset(new Geographic_multilinestring());
      mpy->reset(new Geographic_multipolygon());
      typed_split_gc<Geographic_point, Geographic_linestring,
                     Geographic_polygon, Geographic_geometrycollection,
                     Geographic_multipoint, Geographic_multilinestring,
                     Geographic_multipolygon>(
          down_cast<const Geographic_geometrycollection *>(gc),
          down_cast<Geographic_multipoint *>(mpt->get()),
          down_cast<Geographic_multilinestring *>(mls->get()),
          down_cast<Geographic_multipolygon *>(mpy->get()));
      break;
  }
}

template <typename MPt, typename MLs, typename MPy>
void typed_gc_union(double semi_major, double semi_minor,
                    std::unique_ptr<Multipoint> *mpt,
                    std::unique_ptr<Multilinestring> *mls,
                    std::unique_ptr<Multipolygon> *mpy) {
  Difference difference(semi_major, semi_minor);
  Union union_(semi_major, semi_minor);

  std::unique_ptr<MPy> polygons(new MPy());
  for (auto &py : *down_cast<MPy *>(mpy->get())) {
    polygons.reset(down_cast<MPy *>(union_(polygons.get(), &py)));
    if (polygons->coordinate_system() == Coordinate_system::kGeographic &&
        polygons->is_empty()) {
      // The result of a union between a geographic multipolygon and a
      // geographic polygon is empty. There are two reasons why this may happen:
      //
      // 1. One of the polygons involved are invalid.
      // 2. One of the polygons involved covers half the globe, or more.
      //
      // Since invalid input is only reported to the extent it is explicitly
      // detected, we can simply return a too large polygon error in both cases.
      throw too_large_polygon_exception();
    }
  }

  std::unique_ptr<MLs> linestrings(new MLs());
  linestrings.reset(down_cast<MLs *>(difference(mls->get(), polygons.get())));

  std::unique_ptr<MPt> points(down_cast<MPt *>(
      difference(down_cast<MPt *>(mpt->get()), linestrings.get())));
  points.reset(down_cast<MPt *>(difference(points.get(), polygons.get())));

  mpy->reset(polygons.release());
  mls->reset(linestrings.release());
  mpt->reset(points.release());
}

void gc_union(double semi_major, double semi_minor,
              std::unique_ptr<Multipoint> *mpt,
              std::unique_ptr<Multilinestring> *mls,
              std::unique_ptr<Multipolygon> *mpy) {
  DBUG_ASSERT(mpt->get() && mls->get() && mpy->get());
  DBUG_ASSERT((*mpt)->coordinate_system() == (*mls)->coordinate_system() &&
              (*mpt)->coordinate_system() == (*mpy)->coordinate_system());
  // We're using empty GCs to detect invalid geometries, so empty geometry
  // collections should be filtered out before calling gc_union.
  DBUG_ASSERT(!(*mpt)->empty() || !(*mls)->empty() || !(*mpy)->empty());

  switch ((*mpt)->coordinate_system()) {
    case Coordinate_system::kCartesian: {
      typed_gc_union<Cartesian_multipoint, Cartesian_multilinestring,
                     Cartesian_multipolygon>(semi_major, semi_minor, mpt, mls,
                                             mpy);
      break;
    }
    case Coordinate_system::kGeographic: {
      typed_gc_union<Geographic_multipoint, Geographic_multilinestring,
                     Geographic_multipolygon>(semi_major, semi_minor, mpt, mls,
                                              mpy);
      break;
    }
  }

  // If all collections are empty, we've encountered at least one invalid
  // geometry.
  if ((*mpt)->empty() && (*mls)->empty() && (*mpy)->empty())
    throw invalid_geometry_exception();

  DBUG_ASSERT(mpt->get() && mls->get() && mpy->get());
  DBUG_ASSERT(!(*mpt)->empty() || !(*mls)->empty() || !(*mpy)->empty());
}

}  // namespace gis
