#ifndef GEOFUNC_INTERNAL_INCLUDED
#define GEOFUNC_INTERNAL_INCLUDED

/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file

  @brief
  This file defines common build blocks of GIS functions.
*/

#include <stddef.h>
#include <boost/concept/usage.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <cmath>
#include <utility>
#include <vector>

#include "sql/gis/srid.h"
#include "sql/gis_bg_traits.h"
#include "sql/item_geofunc.h"
#include "sql/spatial.h"

class String;

#define GIS_ZERO 0.00000000001

extern bool simplify_multi_geometry(String *str, String *result_buffer);

/// A wrapper and interface for all geometry types used here. Make these
/// types as localized as possible. It's used as a type interface.
/// @tparam CoordinateSystemType Coordinate system type, specified using
//          those defined in boost::geometry::cs.
template <typename CoordinateSystemType>
class BG_models {
 public:
  typedef Gis_point Point;
  // An counter-clockwise, closed Polygon type. It can hold open Polygon data,
  // but not clockwise ones, otherwise things can go wrong, e.g. intersection.
  typedef Gis_polygon Polygon;
  typedef Gis_line_string Linestring;
  typedef Gis_multi_point Multipoint;
  typedef Gis_multi_line_string Multilinestring;
  typedef Gis_multi_polygon Multipolygon;

  typedef double Coordinate_type;
  typedef CoordinateSystemType Coordinate_system;
};

namespace bg = boost::geometry;
namespace bgm = boost::geometry::model;
namespace bgcs = boost::geometry::cs;
namespace bgi = boost::geometry::index;
namespace bgm = boost::geometry::model;

typedef bgm::point<double, 2, bgcs::cartesian> BG_point;
typedef bgm::box<BG_point> BG_box;
typedef std::pair<BG_box, size_t> BG_rtree_entry;
typedef std::vector<BG_rtree_entry> BG_rtree_entries;
typedef bgi::rtree<BG_rtree_entry, bgi::quadratic<64>> Rtree_index;
typedef std::vector<BG_rtree_entry> Rtree_result;

inline void make_bg_box(const Geometry *g, BG_box *box) {
  MBR mbr;
  g->envelope(&mbr);
  box->min_corner().set<0>(mbr.xmin);
  box->min_corner().set<1>(mbr.ymin);
  box->max_corner().set<0>(mbr.xmax);
  box->max_corner().set<1>(mbr.ymax);
}

inline bool is_box_valid(const BG_box &box) {
  return !(!std::isfinite(box.min_corner().get<0>()) ||
           !std::isfinite(box.min_corner().get<1>()) ||
           !std::isfinite(box.max_corner().get<0>()) ||
           !std::isfinite(box.max_corner().get<1>()) ||
           box.max_corner().get<0>() < box.min_corner().get<0>() ||
           box.max_corner().get<1>() < box.min_corner().get<1>());
}

/**
  Build an rtree set using a geometry collection.
  @param gl geometry object pointers container.
  @param [out] rtree entries which can be used to build an rtree.
 */
void make_rtree(const BG_geometry_collection::Geometry_list &gl,
                Rtree_index *rtree);

/**
  Build an rtree set using array of Boost.Geometry objects, which are
  components of a multi geometry.
  @param mg the multi geometry.
  @param rtree the rtree to build.
 */
template <typename MultiGeometry>
void make_rtree_bggeom(const MultiGeometry &mg, Rtree_index *rtree);

inline Gis_geometry_collection *empty_collection(String *str,
                                                 gis::srid_t srid) {
  return new Gis_geometry_collection(srid, Geometry::wkb_invalid_type, nullptr,
                                     str);
}

/*
  Check whether a geometry is an empty geometry collection, i.e. one that
  doesn't contain any geometry component of [multi]point or [multi]linestring
  or [multi]polygon type.
  @param g the geometry to check.
  @return true if g is such an empty geometry collection;
          false otherwise.
*/
bool is_empty_geocollection(const Geometry *g);

/*
  Check whether wkbres is the data of an empty geometry collection, i.e. one
  that doesn't contain any geometry component of [multi]point or
  [multi]linestring or [multi]polygon type.

  @param wkbres a piece of geometry data of GEOMETRY format, i.e. an SRID
                prefixing a WKB.
  @return true if wkbres contains such an empty geometry collection;
          false otherwise.
 */
bool is_empty_geocollection(const String &wkbres);

/**
   Less than comparator for points used by BG.
 */
struct bgpt_lt {
  template <typename Point>
  bool operator()(const Point &p1, const Point &p2) const {
    if (p1.template get<0>() != p2.template get<0>())
      return p1.template get<0>() < p2.template get<0>();
    else
      return p1.template get<1>() < p2.template get<1>();
  }
};

/**
   Equals comparator for points used by BG.
 */
struct bgpt_eq {
  template <typename Point>
  bool operator()(const Point &p1, const Point &p2) const {
    return p1.template get<0>() == p2.template get<0>() &&
           p1.template get<1>() == p2.template get<1>();
  }
};

/**
  Utility class, reset specified variable 'valref' to specified 'oldval' when
  val_resetter<valtype> instance is destroyed.
  @tparam Valtype Variable type to reset.
 */
template <typename Valtype>
class Var_resetter {
 private:
  Valtype *valref;
  Valtype oldval;

  // Forbid use, to eliminate a warning: oldval may be used uninitialized.
  Var_resetter(const Var_resetter &o);
  Var_resetter &operator=(const Var_resetter &);

 public:
  Var_resetter() : valref(nullptr) {}

  Var_resetter(Valtype *v, const Valtype &oval) : valref(v), oldval(oval) {}

  ~Var_resetter() {
    if (valref) *valref = oldval;
  }

  void set(Valtype *v, const Valtype &oldval) {
    valref = v;
    this->oldval = oldval;
  }
};

/**
  For every Geometry object write-accessed by a boost geometry function, i.e.
  those passed as out parameter into set operation functions, call this
  function before using the result object's data.

  @param          resbuf_mgr   Tracks the result buffer
  @param [in,out] geout        Geometry object
  @param [in,out] res          GEOMETRY string.

  @return true if an error occurred or if the geometry is an empty
          collection; false if no error occurred.
*/
template <typename BG_geotype>
bool post_fix_result(BG_result_buf_mgr *resbuf_mgr, BG_geotype &geout,
                     String *res);

#endif
