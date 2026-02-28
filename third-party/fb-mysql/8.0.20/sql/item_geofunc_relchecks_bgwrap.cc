/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/item_geofunc_relchecks_bgwrap.h"

#include <algorithm>
#include <boost/concept/usage.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/touches.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <utility>

#include "my_dbug.h"
#include "sql/item_geofunc_internal.h"
#include "sql/spatial.h"

/**
  Dispatcher for 'point WITHIN xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a point.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::point_within_geometry(Geometry *g1, Geometry *g2,
                                               bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  if (gt2 == Geometry::wkb_polygon)
    BGCALL(result, within, Point, g1, Polygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipolygon)
    BGCALL(result, within, Point, g1, Multipolygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_point)
    BGCALL(result, within, Point, g1, Point, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipoint)
    BGCALL(result, within, Point, g1, Multipoint, g2, pnull_value);
  else if (gt2 == Geometry::wkb_linestring)
    BGCALL(result, within, Point, g1, Linestring, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multilinestring)
    BGCALL(result, within, Point, g1, Multilinestring, g2, pnull_value);
  else
    DBUG_ASSERT(false);

  return result;
}

/**
  Dispatcher for 'multipoint WITHIN xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multipoint.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multipoint_within_geometry(Geometry *g1, Geometry *g2,
                                                    bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();
  const void *data_ptr = nullptr;

  Multipoint mpts(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                  g1->get_srid());
  if (gt2 == Geometry::wkb_polygon) {
    data_ptr = g2->normalize_ring_order();
    if (data_ptr == nullptr) {
      my_error(ER_GIS_INVALID_DATA, MYF(0), "st_within");
      *pnull_value = true;
      return result;
    }

    Polygon plg(data_ptr, g2->get_data_size(), g2->get_flags(), g2->get_srid());

    result = multipoint_within_geometry_internal(mpts, plg);
  } else if (gt2 == Geometry::wkb_multipolygon) {
    data_ptr = g2->normalize_ring_order();
    if (data_ptr == nullptr) {
      *pnull_value = true;
      my_error(ER_GIS_INVALID_DATA, MYF(0), "st_within");
      return result;
    }

    Multipolygon mplg(data_ptr, g2->get_data_size(), g2->get_flags(),
                      g2->get_srid());

    /*
      One may want to build the rtree index on mpts when mpts has more
      components than mplg, but then one would have to track the points that
      are already known to be in one of mplg's polygons and avoid checking
      again (which may fail and cause false alarm) for other polygon components.
      Such maintenance brings extra cost and performance test prooves that
      it's not desirable.
      The containers tried for such maintenance including std::vector<bool>,
      std::set<array_index>, mpts[i].set_props().

      Also, even if the mplg has only one polygon, i.e. the worst case for
      building rtree index on mplg, the performance is still very very close to
      the linear search done in multipoint_within_geometry_internal.

      So always build index on mplg as below.
    */
    result = multipoint_within_multipolygon(mpts, mplg);
  } else if (gt2 == Geometry::wkb_point) {
    /* There may be duplicate Points, thus use a set to make them unique*/
    Point_set ptset1(mpts.begin(), mpts.end());
    Point pt(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
             g2->get_srid());
    result =
        ((ptset1.size() == 1) && boost::geometry::equals(*ptset1.begin(), pt));
  } else if (gt2 == Geometry::wkb_multipoint) {
    /* There may be duplicate Points, thus use a set to make them unique*/
    Point_set ptset1(mpts.begin(), mpts.end());
    Multipoint mpts2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                     g2->get_srid());
    Point_set ptset2(mpts2.begin(), mpts2.end());
    Point_vector respts;
    typename Point_vector::iterator endpos;
    respts.resize(std::max(ptset1.size(), ptset2.size()));
    endpos = std::set_intersection(ptset1.begin(), ptset1.end(), ptset2.begin(),
                                   ptset2.end(), respts.begin(), bgpt_lt());
    result = (ptset1.size() == static_cast<size_t>(endpos - respts.begin()));
  } else if (gt2 == Geometry::wkb_linestring) {
    Linestring ls(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                  g2->get_srid());
    result = multipoint_within_geometry_internal(mpts, ls);
  } else if (gt2 == Geometry::wkb_multilinestring) {
    Multilinestring mls(g2->get_data_ptr(), g2->get_data_size(),
                        g2->get_flags(), g2->get_srid());
    /*
      Here we can't separate linestrings of a multilinstring MLS to do within
      check one by one because if N (N > 1) linestrings share the same boundary
      point P, P may or may not be a boundary point of MLS, depending on N%2,
      if N is an even number P is an internal point of MLS, otherwise P is a
      boundary point of MLS.
    */
    result = multipoint_within_geometry_internal(mpts, mls);
  } else
    DBUG_ASSERT(false);

  return result;
}

template <typename Geom_types>
template <typename GeomType>
int BG_wrap<Geom_types>::multipoint_within_geometry_internal(
    const Multipoint &mpts, const GeomType &geom) {
  bool has_inner = false;

  for (typename Multipoint::iterator i = mpts.begin(); i != mpts.end(); ++i) {
    /*
      Checking for intersects is faster than within, so if there is at least
      one point within geom, only check that the rest points intersects geom.
     */
    if (!has_inner && (has_inner = boost::geometry::within(*i, geom))) continue;

    if (!boost::geometry::intersects(*i, geom)) return 0;
  }

  return has_inner;
}

template <typename Geom_types>
int BG_wrap<Geom_types>::multipoint_within_multipolygon(
    const Multipoint &mpts, const Multipolygon &mplgn) {
  bool has_inner = false;

  Rtree_index rtree;
  make_rtree_bggeom(mplgn, &rtree);
  BG_box box;

  for (typename Multipoint::iterator i = mpts.begin(); i != mpts.end(); ++i) {
    bool already_in = false;
    // Search for polygons that may intersect *i point using the rtree index.
    boost::geometry::envelope(*i, box);
    Rtree_index::const_query_iterator j = rtree.qbegin(bgi::intersects(box));
    if (j == rtree.qend()) return 0;
    /*
      All polygons that possibly intersect *i point are given by the
      rtree iteration below.
    */
    for (; j != rtree.qend(); ++j) {
      /*
        Checking for intersects is faster than within, so if there is at least
        one point within geom, only check that the rest points intersects geom.
      */
      const Polygon &plgn = mplgn[j->second];
      /*
        If we don't have a point in mpts that's within mplgn yet,
        check whether *i is within plgn.
        If *i is within plgn, it's already in the multipolygon, so no need
        for more checks.
      */
      if (!has_inner && (has_inner = boost::geometry::within(*i, plgn))) {
        already_in = true;
        break;
      }

      /*
        If we already have a point within mplgn, OR if *i is checked above to
        be not within plgn, check whether *i intersects plgn.
        *i has to intersect one of the components in this loop, otherwise *i
        is out of mplgn.
       */
      if (boost::geometry::intersects(*i, plgn)) {
        already_in = true;
        /*
          It's likely that *i is within another plgn, so only stop the
          iteration if we already have a point that's within the multipolygon,
          in order not to miss the polygon containing *i.
        */
        if (has_inner) break;
      }
    }

    /*
      The *i point isn't within or intersects any polygon of mplgn,
      so mpts isn't within geom.
    */
    if (!already_in) return 0;
  }

  /*
    All points in mpts at least intersects geom, so the result is determined
    by whether there is at least one point in mpts that's within geom.
  */
  return has_inner;
}

template <typename Geom_types>
int BG_wrap<Geom_types>::linestring_within_geometry(Geometry *g1, Geometry *g2,
                                                    bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  if (gt2 == Geometry::wkb_polygon)
    BGCALL(result, within, Linestring, g1, Polygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipolygon)
    BGCALL(result, within, Linestring, g1, Multipolygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_point || gt2 == Geometry::wkb_multipoint)
    return 0;
  else if (gt2 == Geometry::wkb_linestring)
    BGCALL(result, within, Linestring, g1, Linestring, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multilinestring)
    BGCALL(result, within, Linestring, g1, Multilinestring, g2, pnull_value);
  else
    DBUG_ASSERT(false);

  return result;
}

template <typename Geom_types>
int BG_wrap<Geom_types>::multilinestring_within_geometry(Geometry *g1,
                                                         Geometry *g2,
                                                         bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  if (gt2 == Geometry::wkb_polygon)
    BGCALL(result, within, Multilinestring, g1, Polygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipolygon)
    BGCALL(result, within, Multilinestring, g1, Multipolygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_point || gt2 == Geometry::wkb_multipoint)
    return 0;
  else if (gt2 == Geometry::wkb_linestring)
    BGCALL(result, within, Multilinestring, g1, Linestring, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multilinestring)
    BGCALL(result, within, Multilinestring, g1, Multilinestring, g2,
           pnull_value);
  else
    DBUG_ASSERT(false);

  return result;
}

template <typename Geom_types>
int BG_wrap<Geom_types>::polygon_within_geometry(Geometry *g1, Geometry *g2,
                                                 bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  if (gt2 == Geometry::wkb_polygon)
    BGCALL(result, within, Polygon, g1, Polygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipolygon)
    BGCALL(result, within, Polygon, g1, Multipolygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_point || gt2 == Geometry::wkb_multipoint ||
           gt2 == Geometry::wkb_linestring ||
           gt2 == Geometry::wkb_multilinestring)
    return 0;
  else
    DBUG_ASSERT(false);

  return result;
}

template <typename Geom_types>
int BG_wrap<Geom_types>::multipolygon_within_geometry(Geometry *g1,
                                                      Geometry *g2,
                                                      bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  if (gt2 == Geometry::wkb_polygon)
    BGCALL(result, within, Multipolygon, g1, Polygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipolygon)
    BGCALL(result, within, Multipolygon, g1, Multipolygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_point || gt2 == Geometry::wkb_multipoint ||
           gt2 == Geometry::wkb_linestring ||
           gt2 == Geometry::wkb_multilinestring)
    return 0;
  else
    DBUG_ASSERT(false);

  return result;
}

/**
  Dispatcher for 'multipoint EQUALS xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multipoint.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multipoint_equals_geometry(Geometry *g1, Geometry *g2,
                                                    bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_point:
      result = Ifsr::equals_check<Geom_types>(g2, g1, pnull_value);
      break;
    case Geometry::wkb_multipoint: {
      Multipoint mpts1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                       g1->get_srid());
      Multipoint mpts2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());

      Point_set ptset1(mpts1.begin(), mpts1.end());
      Point_set ptset2(mpts2.begin(), mpts2.end());
      result =
          (ptset1.size() == ptset2.size() &&
           std::equal(ptset1.begin(), ptset1.end(), ptset2.begin(), bgpt_eq()));
    } break;
    default:
      result = 0;
      break;
  }
  return result;
}

/**
  Dispatcher for 'multipoint disjoint xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multipoint.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multipoint_disjoint_geometry(Geometry *g1,
                                                      Geometry *g2,
                                                      bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();
  const void *data_ptr = nullptr;

  Multipoint mpts1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                   g1->get_srid());
  switch (gt2) {
    case Geometry::wkb_point:
      result = point_disjoint_geometry(g2, g1, pnull_value);
      break;
    case Geometry::wkb_multipoint: {
      Multipoint mpts2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());
      Point_set ptset1(mpts1.begin(), mpts1.end());
      Point_set ptset2(mpts2.begin(), mpts2.end());
      Point_vector respts;
      typename Point_vector::iterator endpos;
      size_t ptset1sz = ptset1.size(), ptset2sz = ptset2.size();

      respts.resize(ptset1sz > ptset2sz ? ptset1sz : ptset2sz);
      endpos =
          std::set_intersection(ptset1.begin(), ptset1.end(), ptset2.begin(),
                                ptset2.end(), respts.begin(), bgpt_lt());
      result = (endpos == respts.begin());
    } break;
    case Geometry::wkb_polygon: {
      data_ptr = g2->normalize_ring_order();
      if (data_ptr == nullptr) {
        *pnull_value = true;
        my_error(ER_GIS_INVALID_DATA, MYF(0), "st_disjoint");
        return result;
      }

      Polygon plg(data_ptr, g2->get_data_size(), g2->get_flags(),
                  g2->get_srid());
      result = multipoint_disjoint_geometry_internal(mpts1, plg);
    } break;
    case Geometry::wkb_multipolygon: {
      data_ptr = g2->normalize_ring_order();
      if (data_ptr == nullptr) {
        *pnull_value = true;
        my_error(ER_GIS_INVALID_DATA, MYF(0), "st_disjoint");
        return result;
      }

      Multipolygon mplg(data_ptr, g2->get_data_size(), g2->get_flags(),
                        g2->get_srid());
      result = multipoint_disjoint_multi_geometry(mpts1, mplg);
    } break;
    case Geometry::wkb_linestring: {
      Linestring ls(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                    g2->get_srid());
      result = multipoint_disjoint_geometry_internal(mpts1, ls);
    } break;
    case Geometry::wkb_multilinestring: {
      Multilinestring mls(g2->get_data_ptr(), g2->get_data_size(),
                          g2->get_flags(), g2->get_srid());
      result = multipoint_disjoint_multi_geometry(mpts1, mls);
    } break;
    default:
      DBUG_ASSERT(false);
      break;
  }

  return result;
}

template <typename Geom_types>
template <typename Geom_type>
int BG_wrap<Geom_types>::multipoint_disjoint_geometry_internal(
    const Multipoint &mpts, const Geom_type &geom) {
  for (typename Multipoint::iterator i = mpts.begin(); i != mpts.end(); ++i) {
    if (!boost::geometry::disjoint(*i, geom)) return 0;
  }

  return 1;
}

template <typename Geom_types>
template <typename Geom_type>
int BG_wrap<Geom_types>::multipoint_disjoint_multi_geometry(
    const Multipoint &mpts, const Geom_type &geom) {
  Rtree_index rtree;

  // Choose the one with more components to build rtree index on, to get more
  // performance improvement.
  if (mpts.size() > geom.size()) {
    make_rtree_bggeom(mpts, &rtree);
    for (typename Geom_type::iterator j = geom.begin(); j != geom.end(); ++j) {
      BG_box box;
      boost::geometry::envelope(*j, box);

      /*
        For each component *j in geom, find points in mpts who intersect
        with MBR(*j), such points are likely to intersect *j, the rest are
        for sure disjoint *j thus no need to check precisely.
      */
      for (Rtree_index::const_query_iterator i =
               rtree.qbegin(bgi::intersects(box));
           i != rtree.qend(); ++i) {
        /*
          If *i really intersect *j, we have the result as false;
          If no *i intersects *j, *j disjoint mpts.
          And if no *j intersect mpts, we can conclude that mpts disjoint geom.
        */
        if (!boost::geometry::disjoint(mpts[i->second], *j)) return 0;
      }
    }
  } else {
    make_rtree_bggeom(geom, &rtree);
    for (typename Multipoint::iterator j = mpts.begin(); j != mpts.end(); ++j) {
      BG_box box;
      boost::geometry::envelope(*j, box);

      /*
        For each point *j in mpts, find components *i in geom such that
        MBR(*i) intersect *j, such *i are likely to intersect *j, the rest are
        for sure disjoint *j thus no need to check precisely.
      */
      for (Rtree_index::const_query_iterator i =
               rtree.qbegin(bgi::intersects(box));
           i != rtree.qend(); ++i) {
        /*
          If *i really intersect *j, we have the result as false;
          If no *i intersects *j, *j disjoint geom.
          And if no *j intersect geom, we can conclude that mpts disjoint geom.
        */
        if (!boost::geometry::disjoint(geom[i->second], *j)) return 0;
      }
    }
  }

  return 1;
}

/**
  Dispatcher for 'linestring disjoint xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a linestring.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::linestring_disjoint_geometry(Geometry *g1,
                                                      Geometry *g2,
                                                      bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  if (gt2 == Geometry::wkb_linestring)
    BGCALL(result, disjoint, Linestring, g1, Linestring, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multilinestring)
    BGCALL(result, disjoint, Linestring, g1, Multilinestring, g2, pnull_value);
  else if (gt2 == Geometry::wkb_point)
    BGCALL(result, disjoint, Linestring, g1, Point, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipoint)
    result = multipoint_disjoint_geometry(g2, g1, pnull_value);
  else if (gt2 == Geometry::wkb_polygon)
    BGCALL(result, disjoint, Linestring, g1, Polygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipolygon)
    BGCALL(result, disjoint, Linestring, g1, Multipolygon, g2, pnull_value);
  else
    DBUG_ASSERT(false);
  return result;
}

/**
  Dispatcher for 'multilinestring disjoint xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multilinestring.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multilinestring_disjoint_geometry(Geometry *g1,
                                                           Geometry *g2,
                                                           bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  if (gt2 == Geometry::wkb_linestring)
    result =
        BG_wrap<Geom_types>::linestring_disjoint_geometry(g2, g1, pnull_value);
  else if (gt2 == Geometry::wkb_multilinestring)
    BGCALL(result, disjoint, Multilinestring, g1, Multilinestring, g2,
           pnull_value);
  else if (gt2 == Geometry::wkb_point)
    BGCALL(result, disjoint, Multilinestring, g1, Point, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipoint)
    result = multipoint_disjoint_geometry(g2, g1, pnull_value);
  else if (gt2 == Geometry::wkb_polygon)
    BGCALL(result, disjoint, Multilinestring, g1, Polygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipolygon)
    BGCALL(result, disjoint, Multilinestring, g1, Multipolygon, g2,
           pnull_value);
  else
    DBUG_ASSERT(false);

  return result;
}

/**
  Dispatcher for 'point disjoint xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a point.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::point_disjoint_geometry(Geometry *g1, Geometry *g2,
                                                 bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_point:
      BGCALL(result, disjoint, Point, g1, Point, g2, pnull_value);
      break;
    case Geometry::wkb_polygon:
      BGCALL(result, disjoint, Point, g1, Polygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, disjoint, Point, g1, Multipolygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint: {
      Multipoint mpts(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                      g2->get_srid());
      Point pt(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
               g1->get_srid());

      Point_set ptset(mpts.begin(), mpts.end());
      result = (ptset.find(pt) == ptset.end());
    } break;
    case Geometry::wkb_linestring:
      BGCALL(result, disjoint, Point, g1, Linestring, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      BGCALL(result, disjoint, Point, g1, Multilinestring, g2, pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }
  return result;
}

/**
  Dispatcher for 'polygon disjoint xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a polygon.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::polygon_disjoint_geometry(Geometry *g1, Geometry *g2,
                                                   bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_point:
      BGCALL(result, disjoint, Polygon, g1, Point, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint:
      result = multipoint_disjoint_geometry(g2, g1, pnull_value);
      break;
    case Geometry::wkb_polygon:
      BGCALL(result, disjoint, Polygon, g1, Polygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, disjoint, Polygon, g1, Multipolygon, g2, pnull_value);
      break;
    case Geometry::wkb_linestring:
      BGCALL(result, disjoint, Polygon, g1, Linestring, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      BGCALL(result, disjoint, Polygon, g1, Multilinestring, g2, pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }
  return result;
}

/**
  Dispatcher for 'multipolygon disjoint xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multipolygon.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multipolygon_disjoint_geometry(Geometry *g1,
                                                        Geometry *g2,
                                                        bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_point:
      BGCALL(result, disjoint, Multipolygon, g1, Point, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint:
      result = multipoint_disjoint_geometry(g2, g1, pnull_value);
      break;
    case Geometry::wkb_polygon:
      BGCALL(result, disjoint, Multipolygon, g1, Polygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, disjoint, Multipolygon, g1, Multipolygon, g2, pnull_value);
      break;
    case Geometry::wkb_linestring:
      BGCALL(result, disjoint, Multipolygon, g1, Linestring, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      BGCALL(result, disjoint, Multipolygon, g1, Multilinestring, g2,
             pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }

  return result;
}

/**
  Dispatcher for 'point intersects xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a point.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::point_intersects_geometry(Geometry *g1, Geometry *g2,
                                                   bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_point:
      BGCALL(result, intersects, Point, g1, Point, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint:
    case Geometry::wkb_linestring:
    case Geometry::wkb_multilinestring:
      result = !point_disjoint_geometry(g1, g2, pnull_value);
      break;
    case Geometry::wkb_polygon:
      BGCALL(result, intersects, Point, g1, Polygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, intersects, Point, g1, Multipolygon, g2, pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }
  return result;
}

/**
  Dispatcher for 'multipoint intersects xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multipoint.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multipoint_intersects_geometry(Geometry *g1,
                                                        Geometry *g2,
                                                        bool *pnull_value) {
  return !multipoint_disjoint_geometry(g1, g2, pnull_value);
}

/**
  Dispatcher for 'linestring intersects xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a linestring.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::linestring_intersects_geometry(Geometry *g1,
                                                        Geometry *g2,
                                                        bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  if (gt2 == Geometry::wkb_point)
    BGCALL(result, intersects, Linestring, g1, Point, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipoint)
    result = multipoint_intersects_geometry(g2, g1, pnull_value);
  else if (gt2 == Geometry::wkb_linestring)
    BGCALL(result, intersects, Linestring, g1, Linestring, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multilinestring)
    BGCALL(result, intersects, Linestring, g1, Multilinestring, g2,
           pnull_value);
  else if (gt2 == Geometry::wkb_polygon)
    BGCALL(result, intersects, Linestring, g1, Polygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipolygon)
    BGCALL(result, intersects, Linestring, g1, Multipolygon, g2, pnull_value);
  else
    DBUG_ASSERT(false);

  return result;
}

/**
  Dispatcher for 'multilinestring intersects xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multilinestring.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multilinestring_intersects_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  if (gt2 == Geometry::wkb_point)
    BGCALL(result, intersects, Multilinestring, g1, Point, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipoint)
    result = multipoint_intersects_geometry(g2, g1, pnull_value);
  else if (gt2 == Geometry::wkb_linestring)
    BGCALL(result, intersects, Multilinestring, g1, Linestring, g2,
           pnull_value);
  else if (gt2 == Geometry::wkb_multilinestring)
    BGCALL(result, intersects, Multilinestring, g1, Multilinestring, g2,
           pnull_value);
  else if (gt2 == Geometry::wkb_polygon)
    BGCALL(result, intersects, Multilinestring, g1, Polygon, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multipolygon)
    BGCALL(result, intersects, Multilinestring, g1, Multipolygon, g2,
           pnull_value);
  else
    DBUG_ASSERT(false);

  return result;
}

/**
  Dispatcher for 'polygon intersects xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a polygon.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::polygon_intersects_geometry(Geometry *g1, Geometry *g2,
                                                     bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_point:
      BGCALL(result, intersects, Polygon, g1, Point, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint:
      result = !multipoint_disjoint_geometry(g2, g1, pnull_value);
      break;
    case Geometry::wkb_polygon:
      BGCALL(result, intersects, Polygon, g1, Polygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, intersects, Polygon, g1, Multipolygon, g2, pnull_value);
      break;
    case Geometry::wkb_linestring:
      BGCALL(result, intersects, Polygon, g1, Linestring, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      BGCALL(result, intersects, Polygon, g1, Multilinestring, g2, pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }

  return result;
}

/**
  Dispatcher for 'multipolygon intersects xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multipolygon.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multipolygon_intersects_geometry(Geometry *g1,
                                                          Geometry *g2,
                                                          bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_point:
      BGCALL(result, intersects, Multipolygon, g1, Point, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint:
      result = !multipoint_disjoint_geometry(g2, g1, pnull_value);
      break;
    case Geometry::wkb_polygon:
      BGCALL(result, intersects, Multipolygon, g1, Polygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, intersects, Multipolygon, g1, Multipolygon, g2,
             pnull_value);
      break;
    case Geometry::wkb_linestring:
      BGCALL(result, intersects, Multipolygon, g1, Linestring, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      BGCALL(result, intersects, Multipolygon, g1, Multilinestring, g2,
             pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }
  return result;
}

/**
  Dispatcher for 'linestring crosses xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a linestring.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::linestring_crosses_geometry(Geometry *g1, Geometry *g2,
                                                     bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_linestring:
      BGCALL(result, crosses, Linestring, g1, Linestring, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      BGCALL(result, crosses, Linestring, g1, Multilinestring, g2, pnull_value);
      break;
    case Geometry::wkb_polygon:
      BGCALL(result, crosses, Linestring, g1, Polygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, crosses, Linestring, g1, Multipolygon, g2, pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }

  return result;
}

/**
  Dispatcher for 'multilinestring crosses xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multilinestring.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multilinestring_crosses_geometry(Geometry *g1,
                                                          Geometry *g2,
                                                          bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_linestring:
      BGCALL(result, crosses, Multilinestring, g1, Linestring, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      BGCALL(result, crosses, Multilinestring, g1, Multilinestring, g2,
             pnull_value);
      break;
    case Geometry::wkb_polygon:
      BGCALL(result, crosses, Multilinestring, g1, Polygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, crosses, Multilinestring, g1, Multipolygon, g2,
             pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }
  return result;
}

/**
  Dispatcher for 'multipoint crosses xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multipoint.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multipoint_crosses_geometry(Geometry *g1, Geometry *g2,
                                                     bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_linestring:
    case Geometry::wkb_multilinestring:
    case Geometry::wkb_polygon:
    case Geometry::wkb_multipolygon: {
      bool has_in = false, has_out = false;
      int res = 0;

      Multipoint mpts(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                      g1->get_srid());
      /*
        According to OGC's definition to crosses, if some Points of
        g1 is in g2 and some are not, g1 crosses g2, otherwise not.
       */
      for (typename Multipoint::iterator i = mpts.begin();
           i != mpts.end() && !(has_in && has_out); ++i) {
        if (!has_out) {
          res = point_disjoint_geometry(&(*i), g2, pnull_value);

          if (!*pnull_value) {
            has_out = res;
            if (has_out) continue;
          } else
            return 0;
        }

        if (!has_in) {
          res = point_within_geometry(&(*i), g2, pnull_value);
          if (!*pnull_value)
            has_in = res;
          else
            return 0;
        }
      }

      result = (has_in && has_out);
    } break;
    default:
      DBUG_ASSERT(false);
      break;
  }

  return result;
}

/**
  Dispatcher for 'multipoint crosses xxx'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multipoint.
  @param g2 Second Geometry operand, not a geometry collection.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multipoint_overlaps_multipoint(Geometry *g1,
                                                        Geometry *g2) {
  int result = 0;

  Multipoint mpts1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                   g1->get_srid());
  Multipoint mpts2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                   g2->get_srid());
  Point_set ptset1, ptset2;

  ptset1.insert(mpts1.begin(), mpts1.end());
  ptset2.insert(mpts2.begin(), mpts2.end());

  // They overlap if they intersect and also each has some points that the other
  // one doesn't have.
  Point_vector respts;
  typename Point_vector::iterator endpos;
  size_t ptset1sz = ptset1.size(), ptset2sz = ptset2.size(), resptssz;

  respts.resize(ptset1sz > ptset2sz ? ptset1sz : ptset2sz);
  endpos = std::set_intersection(ptset1.begin(), ptset1.end(), ptset2.begin(),
                                 ptset2.end(), respts.begin(), bgpt_lt());
  resptssz = endpos - respts.begin();
  if (resptssz > 0 && resptssz < ptset1.size() && resptssz < ptset2.size())
    result = 1;
  else
    result = 0;

  return result;
}

/**
  Dispatcher for 'multilinestring touches polygon'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multilinestring.
  @param g2 Second Geometry operand, a polygon.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multilinestring_touches_polygon(Geometry *g1,
                                                         Geometry *g2,
                                                         bool *pnull_value) {
  const void *data_ptr = g2->normalize_ring_order();
  if (data_ptr == nullptr) {
    *pnull_value = true;
    my_error(ER_GIS_INVALID_DATA, MYF(0), "st_touches");
    return 0;
  }

  Polygon plgn(data_ptr, g2->get_data_size(), g2->get_flags(), g2->get_srid());
  Multilinestring mls(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                      g1->get_srid());

  Multipolygon mplgn;
  mplgn.push_back(plgn);

  int result = boost::geometry::touches(mls, mplgn);

  return result;
}

/**
  Dispatcher for 'point touches geometry'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a point.
  @param g2 Second Geometry operand, a geometry other than geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::point_touches_geometry(Geometry *g1, Geometry *g2,
                                                bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_linestring:
      BGCALL(result, touches, Point, g1, Linestring, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      BGCALL(result, touches, Point, g1, Multilinestring, g2, pnull_value);
      break;
    case Geometry::wkb_polygon:
      BGCALL(result, touches, Point, g1, Polygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, touches, Point, g1, Multipolygon, g2, pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }

  return result;
}

/**
  Dispatcher for 'multipoint touches geometry'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multipoint.
  @param g2 Second Geometry operand, a geometry other than geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multipoint_touches_geometry(Geometry *g1, Geometry *g2,
                                                     bool *pnull_value) {
  int has_touches = 0;

  Multipoint mpts(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                  g1->get_srid());
  for (typename Multipoint::iterator i = mpts.begin(); i != mpts.end(); ++i) {
    int ptg = point_touches_geometry(&(*i), g2, pnull_value);
    if (*pnull_value) return 0;
    if (ptg)
      has_touches = 1;
    else if (!point_disjoint_geometry(&(*i), g2, pnull_value))
      return 0;
  }

  return has_touches;
}

/**
  Dispatcher for 'linestring touches geometry'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a linestring.
  @param g2 Second Geometry operand, a geometry other than geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::linestring_touches_geometry(Geometry *g1, Geometry *g2,
                                                     bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_point:
      BGCALL(result, touches, Linestring, g1, Point, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint:
      result = multipoint_touches_geometry(g2, g1, pnull_value);
      break;
    case Geometry::wkb_linestring:
      BGCALL(result, touches, Linestring, g1, Linestring, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      BGCALL(result, touches, Linestring, g1, Multilinestring, g2, pnull_value);
      break;
    case Geometry::wkb_polygon:
      BGCALL(result, touches, Linestring, g1, Polygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, touches, Linestring, g1, Multipolygon, g2, pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }
  return result;
}

/**
  Dispatcher for 'multilinestring touches geometry'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multilinestring.
  @param g2 Second Geometry operand, a geometry other than geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multilinestring_touches_geometry(Geometry *g1,
                                                          Geometry *g2,
                                                          bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_point:
      BGCALL(result, touches, Multilinestring, g1, Point, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint:
      result = multipoint_touches_geometry(g2, g1, pnull_value);
      break;
    case Geometry::wkb_linestring:
      BGCALL(result, touches, Multilinestring, g1, Linestring, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      BGCALL(result, touches, Multilinestring, g1, Multilinestring, g2,
             pnull_value);
      break;
    case Geometry::wkb_polygon:
      result = BG_wrap<Geom_types>::multilinestring_touches_polygon(
          g1, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, touches, Multilinestring, g1, Multipolygon, g2,
             pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }
  return result;
}

/**
  Dispatcher for 'polygon touches geometry'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a polygon.
  @param g2 Second Geometry operand, a geometry other than geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::polygon_touches_geometry(Geometry *g1, Geometry *g2,
                                                  bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_point:
      BGCALL(result, touches, Polygon, g1, Point, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint:
      result = multipoint_touches_geometry(g2, g1, pnull_value);
      break;
    case Geometry::wkb_polygon:
      BGCALL(result, touches, Polygon, g1, Polygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, touches, Polygon, g1, Multipolygon, g2, pnull_value);
      break;
    case Geometry::wkb_linestring:
      BGCALL(result, touches, Polygon, g1, Linestring, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      result = BG_wrap<Geom_types>::multilinestring_touches_polygon(
          g2, g1, pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }
  return result;
}

/**
  Dispatcher for 'multipolygon touches geometry'.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, a multipolygon.
  @param g2 Second Geometry operand, a geometry other than geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
template <typename Geom_types>
int BG_wrap<Geom_types>::multipolygon_touches_geometry(Geometry *g1,
                                                       Geometry *g2,
                                                       bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt2) {
    case Geometry::wkb_point:
      BGCALL(result, touches, Multipolygon, g1, Point, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint:
      result = multipoint_touches_geometry(g2, g1, pnull_value);
      break;
    case Geometry::wkb_polygon:
      BGCALL(result, touches, Multipolygon, g1, Polygon, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      BGCALL(result, touches, Multipolygon, g1, Multipolygon, g2, pnull_value);
      break;
    case Geometry::wkb_linestring:
      BGCALL(result, touches, Multipolygon, g1, Linestring, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      BGCALL(result, touches, Multipolygon, g1, Multilinestring, g2,
             pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }

  return result;
}

/*
  Explicit template instantiations are needed here.
  Templates were moved to separate file in order to avoid
  inlining and out-of-memory problems in optmized mode on gcc/solaris/intel.
 */
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::point_within_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::multipoint_within_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::linestring_within_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multilinestring_within_geometry(Geometry *g1, Geometry *g2,
                                    bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::polygon_within_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multipolygon_within_geometry(Geometry *g1, Geometry *g2, bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::multipoint_equals_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::point_disjoint_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multipoint_disjoint_geometry(Geometry *g1, Geometry *g2, bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    linestring_disjoint_geometry(Geometry *g1, Geometry *g2, bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multilinestring_disjoint_geometry(Geometry *g1, Geometry *g2,
                                      bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::polygon_disjoint_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multipolygon_disjoint_geometry(Geometry *g1, Geometry *g2,
                                   bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::point_intersects_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multipoint_intersects_geometry(Geometry *g1, Geometry *g2,
                                   bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    linestring_intersects_geometry(Geometry *g1, Geometry *g2,
                                   bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multilinestring_intersects_geometry(Geometry *g1, Geometry *g2,
                                        bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::polygon_intersects_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multipolygon_intersects_geometry(Geometry *g1, Geometry *g2,
                                     bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::linestring_crosses_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::multipoint_crosses_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multilinestring_crosses_geometry(Geometry *g1, Geometry *g2,
                                     bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multipoint_overlaps_multipoint(Geometry *g1, Geometry *g2);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::point_touches_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::multipoint_touches_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::linestring_touches_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multilinestring_touches_polygon(Geometry *g1, Geometry *g2,
                                    bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multilinestring_touches_geometry(Geometry *g1, Geometry *g2,
                                     bool *pnull_value);
template int
BG_wrap<BG_models<boost::geometry::cs::cartesian>>::polygon_touches_geometry(
    Geometry *g1, Geometry *g2, bool *pnull_value);
template int BG_wrap<BG_models<boost::geometry::cs::cartesian>>::
    multipolygon_touches_geometry(Geometry *g1, Geometry *g2,
                                  bool *pnull_value);
