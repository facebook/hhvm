/* Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef ITEM_GEOFUNC_BGWRAP_INCLUDED
#define ITEM_GEOFUNC_BGWRAP_INCLUDED

/**
  Wraps and dispatches type specific BG function calls according to operation
  type and both operands' types.

  We want to isolate boost header file inclusion only inside this file, so we
  put this class declaration in an internal header file. And we want to make the
  methods static since no state is needed here.

  @tparam Geom_types Geometry types definitions.
*/

#include <stddef.h>
#include <string.h>  // Boost expects ::memset to be already present.
#include <set>
#include <string>  // Boost expects std::string to be already present.
#include <vector>

#include <boost/geometry/algorithms/crosses.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/touches.hpp>
#include <boost/geometry/algorithms/within.hpp>

#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql/item_geofunc.h"

class Geometry;
struct bgpt_lt;

template <typename Geom_types>
class BG_wrap {
 public:
  typedef typename Geom_types::Point Point;
  typedef typename Geom_types::Linestring Linestring;
  typedef typename Geom_types::Polygon Polygon;
  typedef typename Geom_types::Multipoint Multipoint;
  typedef typename Geom_types::Multilinestring Multilinestring;
  typedef typename Geom_types::Multipolygon Multipolygon;
  typedef typename Geom_types::Coordinate_type Coord_type;
  typedef typename Geom_types::Coordinate_system Coordsys;

  // For abbrievation.
  typedef Item_func_spatial_rel Ifsr;
  typedef std::set<Point, bgpt_lt> Point_set;
  typedef std::vector<Point> Point_vector;

  static int point_within_geometry(Geometry *g1, Geometry *g2,
                                   bool *pnull_value);

  static int multipoint_within_geometry(Geometry *g1, Geometry *g2,
                                        bool *pnull_value);

  static int linestring_within_geometry(Geometry *g1, Geometry *g2,
                                        bool *pnull_value);
  static int multilinestring_within_geometry(Geometry *g1, Geometry *g2,
                                             bool *pnull_value);
  static int polygon_within_geometry(Geometry *g1, Geometry *g2,
                                     bool *pnull_value);
  static int multipolygon_within_geometry(Geometry *g1, Geometry *g2,
                                          bool *pnull_value);

  static int multipoint_equals_geometry(Geometry *g1, Geometry *g2,
                                        bool *pnull_value);

  static int point_disjoint_geometry(Geometry *g1, Geometry *g2,
                                     bool *pnull_value);
  static int multipoint_disjoint_geometry(Geometry *g1, Geometry *g2,
                                          bool *pnull_value);

  static int linestring_disjoint_geometry(Geometry *g1, Geometry *g2,
                                          bool *pnull_value);
  static int multilinestring_disjoint_geometry(Geometry *g1, Geometry *g2,
                                               bool *pnull_value);
  static int polygon_disjoint_geometry(Geometry *g1, Geometry *g2,
                                       bool *pnull_value);
  static int multipolygon_disjoint_geometry(Geometry *g1, Geometry *g2,
                                            bool *pnull_value);
  static int point_intersects_geometry(Geometry *g1, Geometry *g2,
                                       bool *pnull_value);
  static int multipoint_intersects_geometry(Geometry *g1, Geometry *g2,
                                            bool *pnull_value);
  static int linestring_intersects_geometry(Geometry *g1, Geometry *g2,
                                            bool *pnull_value);
  static int multilinestring_intersects_geometry(Geometry *g1, Geometry *g2,
                                                 bool *pnull_value);
  static int polygon_intersects_geometry(Geometry *g1, Geometry *g2,
                                         bool *pnull_value);
  static int multipolygon_intersects_geometry(Geometry *g1, Geometry *g2,
                                              bool *pnull_value);
  static int linestring_crosses_geometry(Geometry *g1, Geometry *g2,
                                         bool *pnull_value);
  static int multipoint_crosses_geometry(Geometry *g1, Geometry *g2,
                                         bool *pnull_value);
  static int multilinestring_crosses_geometry(Geometry *g1, Geometry *g2,
                                              bool *pnull_value);
  static int multipoint_overlaps_multipoint(Geometry *g1, Geometry *g2);
  static int point_touches_geometry(Geometry *g1, Geometry *g2,
                                    bool *pnull_value);
  static int multipoint_touches_geometry(Geometry *g1, Geometry *g2,
                                         bool *pnull_value);
  static int linestring_touches_geometry(Geometry *g1, Geometry *g2,
                                         bool *pnull_value);
  static int multilinestring_touches_polygon(Geometry *g1, Geometry *g2,
                                             bool *pnull_value);
  static int multilinestring_touches_geometry(Geometry *g1, Geometry *g2,
                                              bool *pnull_value);
  static int polygon_touches_geometry(Geometry *g1, Geometry *g2,
                                      bool *pnull_value);
  static int multipolygon_touches_geometry(Geometry *g1, Geometry *g2,
                                           bool *pnull_value);

 private:
  template <typename Geom_type>
  static int multipoint_disjoint_geometry_internal(const Multipoint &mpts1,
                                                   const Geom_type &geom);
  template <typename Geom_type>
  static int multipoint_disjoint_multi_geometry(const Multipoint &mpts,
                                                const Geom_type &geom);
  template <typename GeomType>
  static int multipoint_within_geometry_internal(const Multipoint &mpts,
                                                 const GeomType &geom);
  static int multipoint_within_multipolygon(const Multipoint &mpts,
                                            const Multipolygon &mplgn);
};  // BG_wrap

/*
  Call a BG function with specified types of operands. We have to create
  geo1 and geo2 because operands g1 and g2 are created without their WKB data
  parsed, so not suitable for BG to use. geo1 will share the same copy of WKB
  data with g1, also true for geo2.
 */
#define BGCALL(res, bgfunc, GeoType1, g1, GeoType2, g2, pnullval) \
  do {                                                            \
    const void *pg1 = g1->normalize_ring_order();                 \
    const void *pg2 = g2->normalize_ring_order();                 \
    if (pg1 != NULL && pg2 != NULL) {                             \
      GeoType1 geo1(pg1, g1->get_data_size(), g1->get_flags(),    \
                    g1->get_srid());                              \
      GeoType2 geo2(pg2, g2->get_data_size(), g2->get_flags(),    \
                    g2->get_srid());                              \
      res = boost::geometry::bgfunc(geo1, geo2);                  \
    } else {                                                      \
      my_error(ER_GIS_INVALID_DATA, MYF(0), "st_" #bgfunc);       \
      (*(pnullval)) = 1;                                          \
    }                                                             \
  } while (0)

#endif  // ITEM_GEOFUNC_BGWRAP_INCLUDED
