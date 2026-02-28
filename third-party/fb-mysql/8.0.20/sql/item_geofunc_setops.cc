/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include <stddef.h>
#include <string.h>
#include <algorithm>
#include <boost/concept/usage.hpp>
#include <boost/geometry/algorithms/difference.hpp>
#include <boost/geometry/algorithms/sym_difference.hpp>
#include <boost/geometry/algorithms/union.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <iterator>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "m_ctype.h"
#include "m_string.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/psi/psi_base.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/derror.h"  // ER_THD
#include "sql/gis/srid.h"
#include "sql/inplace_vector.h"
#include "sql/item.h"
#include "sql/item_func.h"
#include "sql/item_geofunc.h"
#include "sql/item_geofunc_internal.h"
#include "sql/spatial.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_error.h"
#include "sql/sql_exception_handler.h"
#include "sql/srs_fetcher.h"
#include "sql_string.h"
#include "template_utils.h"

template <typename Geom_types>
class BG_setop_wrapper;

namespace boost {
namespace geometry {
namespace cs {
struct cartesian;
}  // namespace cs
}  // namespace geometry
}  // namespace boost

#define BGOPCALL(GeoOutType, geom_out, bgop, GeoType1, g1, GeoType2, g2,       \
                 wkbres, nullval)                                              \
  do {                                                                         \
    const void *pg1 = g1->normalize_ring_order();                              \
    const void *pg2 = g2->normalize_ring_order();                              \
    geom_out = NULL;                                                           \
    if (pg1 != NULL && pg2 != NULL) {                                          \
      GeoType1 geo1(pg1, g1->get_data_size(), g1->get_flags(),                 \
                    g1->get_srid());                                           \
      GeoType2 geo2(pg2, g2->get_data_size(), g2->get_flags(),                 \
                    g2->get_srid());                                           \
      std::unique_ptr<GeoOutType> geout(new GeoOutType());                     \
      geout->set_srid(g1->get_srid());                                         \
      boost::geometry::bgop(geo1, geo2, *geout);                               \
      (nullval) = false;                                                       \
      if (geout->size() == 0 ||                                                \
          (nullval =                                                           \
               post_fix_result(&(m_ifso->m_bg_resbuf_mgr), *geout, wkbres))) { \
        if (nullval) return NULL;                                              \
      } else                                                                   \
        geom_out = geout.release();                                            \
    } else {                                                                   \
      (nullval) = true;                                                        \
      my_error(ER_GIS_INVALID_DATA, MYF(0), "st_" #bgop);                      \
      return NULL;                                                             \
    }                                                                          \
  } while (0)

/*
  Write an empty geometry collection's wkb encoding into str, and create a
  geometry object for this empty geometry colletion.
 */
Geometry *Item_func_spatial_operation::empty_result(String *str,
                                                    gis::srid_t srid) {
  if ((null_value = str->reserve(GEOM_HEADER_SIZE + 4 + 16, 256)))
    return nullptr;

  write_geometry_header(str, srid, Geometry::wkb_geometrycollection, 0);
  Gis_geometry_collection *gcol = new Gis_geometry_collection();
  gcol->set_data_ptr(str->ptr() + GEOM_HEADER_SIZE, 4);
  gcol->has_geom_header_space(true);
  return gcol;
}

/**
  Wraps and dispatches type specific BG function calls according to operation
  type and the 1st or both operand type(s), depending on code complexity.

  We want to isolate boost header file inclusion only inside this file, so we
  can't put this class declaration in any header file. And we want to make the
  methods static since no state is needed here.
  @tparam Geom_types A wrapper for all geometry types.
*/
template <typename Geom_types>
class BG_setop_wrapper {
  // Some computation in this class may rely on functions in
  // Item_func_spatial_operation.
  Item_func_spatial_operation *m_ifso;
  bool null_value;  // Whether computation has error.

  // Some computation in this class may rely on functions in
  // Item_func_spatial_operation, after each call of its functions, copy its
  // null_value, we don't want to miss errors.
  void copy_ifso_state() { null_value = m_ifso->null_value; }

 public:
  typedef typename Geom_types::Point Point;
  typedef typename Geom_types::Linestring Linestring;
  typedef typename Geom_types::Polygon Polygon;
  typedef typename Geom_types::Polygon::ring_type Polygon_ring;
  typedef typename Geom_types::Multipoint Multipoint;
  typedef typename Geom_types::Multilinestring Multilinestring;
  typedef typename Geom_types::Multipolygon Multipolygon;
  typedef typename Geom_types::Coordinate_type Coord_type;
  typedef typename Geom_types::Coordinate_system Coordsys;
  typedef Item_func_spatial_rel Ifsr;
  typedef Item_func_spatial_operation Ifso;
  typedef std::set<Point, bgpt_lt> Point_set;
  typedef std::vector<Point> Point_vector;

  BG_setop_wrapper(Item_func_spatial_operation *ifso) {
    m_ifso = ifso;
    null_value = 0;
  }

  bool get_null_value() const { return null_value; }

  /**
    Do point insersection point operation.
    @param g1 First geometry operand, must be a point.
    @param g2 Second geometry operand, must be a point.
    @param[out] result Holds WKB data of the result.
    @return the result Geometry whose WKB data is in result.
    */
  Geometry *point_intersection_point(Geometry *g1, Geometry *g2,
                                     String *result) {
    Geometry *retgeo = nullptr;

    Point pt1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
              g1->get_srid());
    Point pt2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
              g2->get_srid());

    if (bgpt_eq()(pt1, pt2)) {
      retgeo = g1;
      null_value = retgeo->as_geometry(result, true);
    } else {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }

  /*
    Do point intersection Multipoint operation.
    The parameters and return value has identical/similar meaning as to above
    function, which can be inferred from the function name, we won't repeat
    here or for the rest of the functions in this class.
  */
  Geometry *point_intersection_multipoint(Geometry *g1, Geometry *g2,
                                          String *result) {
    Geometry *retgeo = nullptr;

    Point pt(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
             g1->get_srid());
    Multipoint mpts(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                    g2->get_srid());
    Point_set ptset(mpts.begin(), mpts.end());

    if (ptset.find(pt) != ptset.end()) {
      retgeo = g1;
      null_value = retgeo->as_geometry(result, true);
    } else {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }

  Geometry *point_intersection_geometry(Geometry *g1, Geometry *g2,
                                        String *result) {
#if !defined(DBUG_OFF)
    Geometry::wkbType gt2 = g2->get_type();
#endif
    Geometry *retgeo = nullptr;

    bool is_out = !Ifsr::bg_geo_relation_check(g1, g2, Ifsr::SP_DISJOINT_FUNC,
                                               &null_value);

    DBUG_ASSERT(gt2 == Geometry::wkb_linestring ||
                gt2 == Geometry::wkb_polygon ||
                gt2 == Geometry::wkb_multilinestring ||
                gt2 == Geometry::wkb_multipolygon);
    if (!null_value) {
      if (is_out) {
        null_value = g1->as_geometry(result, true);
        retgeo = g1;
      } else {
        retgeo = m_ifso->empty_result(result, g1->get_srid());
        copy_ifso_state();
      }
    }
    return retgeo;
  }

  Geometry *multipoint_intersection_multipoint(Geometry *g1, Geometry *g2,
                                               String *result) {
    Geometry *retgeo = nullptr;
    Point_set ptset1, ptset2;
    Multipoint *mpts = new Multipoint();
    std::unique_ptr<Multipoint> guard(mpts);

    mpts->set_srid(g1->get_srid());

    Multipoint mpts1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                     g1->get_srid());
    Multipoint mpts2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                     g2->get_srid());

    ptset1.insert(mpts1.begin(), mpts1.end());
    ptset2.insert(mpts2.begin(), mpts2.end());

    Point_vector respts;
    typename Point_vector::iterator endpos;
    size_t ptset1sz = ptset1.size(), ptset2sz = ptset2.size();
    respts.resize(ptset1sz > ptset2sz ? ptset1sz : ptset2sz);

    endpos = std::set_intersection(ptset1.begin(), ptset1.end(), ptset2.begin(),
                                   ptset2.end(), respts.begin(), bgpt_lt());
    std::copy(respts.begin(), endpos, std::back_inserter(*mpts));
    if (mpts->size() > 0) {
      null_value = m_ifso->assign_result(mpts, result);
      retgeo = mpts;
      guard.release();
    } else {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }

    return retgeo;
  }

  Geometry *multipoint_intersection_geometry(Geometry *g1, Geometry *g2,
                                             String *result) {
    Geometry *retgeo = nullptr;
#if !defined(DBUG_OFF)
    Geometry::wkbType gt2 = g2->get_type();
#endif
    Point_set ptset;
    Multipoint mpts(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                    g1->get_srid());
    Multipoint *mpts2 = new Multipoint();
    std::unique_ptr<Multipoint> guard(mpts2);

    mpts2->set_srid(g1->get_srid());

    DBUG_ASSERT(gt2 == Geometry::wkb_linestring ||
                gt2 == Geometry::wkb_polygon ||
                gt2 == Geometry::wkb_multilinestring ||
                gt2 == Geometry::wkb_multipolygon);
    ptset.insert(mpts.begin(), mpts.end());

    for (typename Point_set::iterator i = ptset.begin(); i != ptset.end();
         ++i) {
      Point &pt = const_cast<Point &>(*i);
      if (!Ifsr::bg_geo_relation_check(&pt, g2, Ifsr::SP_DISJOINT_FUNC,
                                       &null_value) &&
          !null_value) {
        mpts2->push_back(pt);
      }

      if (null_value) return nullptr;
    }

    if (mpts2->size() > 0) {
      null_value = m_ifso->assign_result(mpts2, result);
      retgeo = mpts2;
      guard.release();
    } else {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }

    return retgeo;
  }

  Geometry *linestring_intersection_linestring(Geometry *g1, Geometry *g2,
                                               String *result) {
    Multilinestring *res = nullptr;
    Geometry *retgeo = nullptr;

    // BG will return all intersection lines and points in a
    // multilinestring. Intersection points are represented as lines
    // with the same start and end points.
    BGOPCALL(Multilinestring, res, intersection, Linestring, g1, Linestring, g2,
             nullptr, null_value);

    if (res == nullptr) return m_ifso->empty_result(result, g1->get_srid());

    retgeo = m_ifso->simplify_multilinestring(res, result);
    delete res;
    return retgeo;
  }

  Geometry *linestring_intersection_polygon(Geometry *g1, Geometry *g2,
                                            String *result) {
    Geometry::wkbType gt2 = g2->get_type();
    Geometry *retgeo = nullptr, *tmp1 = nullptr, *tmp2 = nullptr;
    // It is likely for there to be discrete intersection Points.
    if (gt2 == Geometry::wkb_multipolygon) {
      BGOPCALL(Multilinestring, tmp1, intersection, Linestring, g1,
               Multipolygon, g2, nullptr, null_value);
      BGOPCALL(Multipoint, tmp2, intersection, Linestring, g1, Multipolygon, g2,
               nullptr, null_value);
    } else {
      BGOPCALL(Multilinestring, tmp1, intersection, Linestring, g1, Polygon, g2,
               nullptr, null_value);
      BGOPCALL(Multipoint, tmp2, intersection, Linestring, g1, Polygon, g2,
               nullptr, null_value);
    }

    // Need merge, exclude Points that are on the result Linestring.
    retgeo = m_ifso->combine_sub_results<Coordsys>(tmp1, tmp2, g1->get_srid(),
                                                   result);
    copy_ifso_state();

    return retgeo;
  }

  Geometry *linestring_intersection_multilinestring(Geometry *g1, Geometry *g2,
                                                    String *result) {
    Multilinestring *res = nullptr;
    Geometry *retgeo = nullptr;

    // BG will return all intersection lines and points in a
    // multilinestring. Intersection points are represented as lines
    // with the same start and end points.
    BGOPCALL(Multilinestring, res, intersection, Linestring, g1,
             Multilinestring, g2, nullptr, null_value);

    if (res == nullptr || res->size() == 0)
      return m_ifso->empty_result(result, g1->get_srid());

    retgeo = m_ifso->simplify_multilinestring(res, result);
    delete res;
    return retgeo;
  }

  Geometry *polygon_intersection_multilinestring(Geometry *g1, Geometry *g2,
                                                 String *result) {
    Geometry *retgeo = nullptr, *tmp1 = nullptr;
    Multipoint *tmp2 = nullptr;
    std::unique_ptr<Geometry> guard1;

    BGOPCALL(Multilinestring, tmp1, intersection, Polygon, g1, Multilinestring,
             g2, nullptr, null_value);
    guard1.reset(tmp1);

    Multilinestring mlstr(g2->get_data_ptr(), g2->get_data_size(),
                          g2->get_flags(), g2->get_srid());
    Multipoint mpts;
    Point_set ptset;

    const void *data_ptr = g1->normalize_ring_order();
    if (data_ptr == nullptr) {
      null_value = true;
      my_error(ER_GIS_INVALID_DATA, MYF(0), "st_intersection");
      return nullptr;
    }

    Polygon plgn(data_ptr, g1->get_data_size(), g1->get_flags(),
                 g1->get_srid());

    for (typename Multilinestring::iterator i = mlstr.begin(); i != mlstr.end();
         ++i) {
      boost::geometry::intersection(plgn, *i, mpts);
      if (mpts.size() > 0) {
        ptset.insert(mpts.begin(), mpts.end());
        mpts.clear();
      }
    }

    std::unique_ptr<Multipoint> guard2;
    if (ptset.size() > 0) {
      tmp2 = new Multipoint;
      tmp2->set_srid(g1->get_srid());
      guard2.reset(tmp2);
      std::copy(ptset.begin(), ptset.end(), std::back_inserter(*tmp2));
    }

    retgeo = m_ifso->combine_sub_results<Coordsys>(
        guard1.release(), guard2.release(), g1->get_srid(), result);
    copy_ifso_state();

    return retgeo;
  }

  /**
    Compute the multilinestring result if any for intersection of two polygons.
    First get the multilinestrings mls1 and mls2 from polygons, then compute
    mls3 = mls1 intersection mls2, and finally return mls3 - mls4 as result,
    where mls4 is got from rings of the multipolygon intersection result of
    plgn1 and plgn2.

    @param plgn1 the 1st polygon operand
    @param plgn2 the 2nd polygon operand
    @param mplgn the areal intersection result of plgn1 and plgn2
    @param [out] mls the result multilinestring
   */
  template <typename Polygon_type1, typename Polygon_type2>
  void plgn_intersection_plgn_mls(const Polygon_type1 &plgn1,
                                  const Polygon_type2 &plgn2,
                                  const Multipolygon &mplgn,
                                  Multilinestring &mls) {
    if (mplgn.size() > 0) {
      /*
        Remove the linestring parts that are part of the result polygon rings.
        The discrete intersection points that are also removed here.
      */
      Multilinestring mls3;
      bg::intersection(plgn1, plgn2, mls3);
      bg::detail::boundary_view<Multipolygon const> view(mplgn);
      bg::difference(mls3, view, mls);
    } else
      bg::intersection(plgn1, plgn2, mls);
  }

  Geometry *polygon_intersection_polygon(Geometry *g1, Geometry *g2,
                                         String *result) {
    Geometry::wkbType gt2 = g2->get_type();
    Geometry *retgeo = nullptr;

    const void *pg1 = g1->normalize_ring_order();
    const void *pg2 = g2->normalize_ring_order();
    if (pg1 == nullptr || pg2 == nullptr) {
      null_value = true;
      my_error(ER_GIS_INVALID_DATA, MYF(0), "st_intersection");
      return nullptr;
    }

    Multilinestring mls;
    Polygon plgn1(pg1, g1->get_data_size(), g1->get_flags(), g1->get_srid());

    std::unique_ptr<Multipolygon> mplgn_result(new Multipolygon());
    mplgn_result->set_srid(g1->get_srid());

    if (gt2 == Geometry::wkb_polygon) {
      Polygon plgn2(pg2, g2->get_data_size(), g2->get_flags(), g2->get_srid());
      bg::intersection(plgn1, plgn2, *mplgn_result);
      plgn_intersection_plgn_mls(plgn1, plgn2, *mplgn_result, mls);
    } else {
      Multipolygon mplgn2(pg2, g2->get_data_size(), g2->get_flags(),
                          g2->get_srid());
      bg::intersection(plgn1, mplgn2, *mplgn_result);
      plgn_intersection_plgn_mls(plgn1, mplgn2, *mplgn_result, mls);
    }

    retgeo = combine_mls_mplgn_results(&mls, mplgn_result.get(), result);
    if (retgeo == mplgn_result.get()) mplgn_result.release();
    copy_ifso_state();

    return retgeo;
  }

  Geometry *combine_mls_mplgn_results(Multilinestring *mls,
                                      Multipolygon *mplgn_result,
                                      String *result) {
    Geometry *geom = nullptr, *retgeo = nullptr;
    DBUG_ASSERT(mls != nullptr);

    if (mls->size() > 0) geom = m_ifso->simplify_multilinestring(mls, result);

    if (mplgn_result->size() > 0) {
      if (mls->size() == 0) {
        // The multipolygon is the only result.
        DBUG_ASSERT(result->length() == 0);
        null_value =
            post_fix_result(&(m_ifso->m_bg_resbuf_mgr), *mplgn_result, result);
        if (null_value)
          return nullptr;
        else
          retgeo = mplgn_result;
      } else {
        String mplgn_resbuf;
        null_value = post_fix_result(&(m_ifso->m_bg_resbuf_mgr), *mplgn_result,
                                     &mplgn_resbuf);
        if (null_value) return nullptr;
        if (geom->get_type() == Geometry::wkb_geometrycollection) {
          down_cast<Gis_geometry_collection *>(geom)->append_geometry(
              &(*mplgn_result), result);
          retgeo = geom;
        } else {
          /*
            The geom created from mls isn't a GC, so we have to create a GC to
            hole both geom and mplgn_result.
           */
          String tmp_mls_resbuf;
          tmp_mls_resbuf.takeover(*result);

          Gis_geometry_collection *tmp_gc =
              new Gis_geometry_collection(&(*mplgn_result), result);
          tmp_gc->append_geometry(geom, result);
          retgeo = tmp_gc;
          delete geom;
        }
      }
    } else
      retgeo = geom;

    if (retgeo == nullptr)
      retgeo = m_ifso->empty_result(result, mplgn_result->get_srid());

    return retgeo;
  }

  Geometry *multilinestring_intersection_multilinestring(Geometry *g1,
                                                         Geometry *g2,
                                                         String *result) {
    Multilinestring *res = nullptr;
    Geometry *retgeo = nullptr;

    // BG will return all intersection lines and points in a
    // multilinestring. Intersection points are represented as lines
    // with the same start and end points.
    BGOPCALL(Multilinestring, res, intersection, Multilinestring, g1,
             Multilinestring, g2, nullptr, null_value);

    if (res == nullptr) return m_ifso->empty_result(result, g1->get_srid());

    retgeo = m_ifso->simplify_multilinestring(res, result);
    delete res;
    return retgeo;
  }

  Geometry *multilinestring_intersection_multipolygon(Geometry *g1,
                                                      Geometry *g2,
                                                      String *result) {
    Geometry *retgeo = nullptr, *tmp1 = nullptr;
    Multipoint *tmp2 = nullptr;

    std::unique_ptr<Geometry> guard1;
    BGOPCALL(Multilinestring, tmp1, intersection, Multilinestring, g1,
             Multipolygon, g2, nullptr, null_value);
    guard1.reset(tmp1);

    Multilinestring mlstr(g1->get_data_ptr(), g1->get_data_size(),
                          g1->get_flags(), g1->get_srid());
    Multipoint mpts;

    const void *data_ptr = g2->normalize_ring_order();
    if (data_ptr == nullptr) {
      null_value = true;
      my_error(ER_GIS_INVALID_DATA, MYF(0), "st_intersection");
      return nullptr;
    }

    Multipolygon mplgn(data_ptr, g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());
    Point_set ptset;

    for (typename Multilinestring::iterator i = mlstr.begin(); i != mlstr.end();
         ++i) {
      boost::geometry::intersection(*i, mplgn, mpts);
      if (mpts.size() > 0) {
        ptset.insert(mpts.begin(), mpts.end());
        mpts.clear();
      }
    }

    std::unique_ptr<Multipoint> guard2;
    if (ptset.empty() == false) {
      tmp2 = new Multipoint;
      tmp2->set_srid(g1->get_srid());
      guard2.reset(tmp2);
      std::copy(ptset.begin(), ptset.end(), std::back_inserter(*tmp2));
    }

    retgeo = m_ifso->combine_sub_results<Coordsys>(
        guard1.release(), guard2.release(), g1->get_srid(), result);
    copy_ifso_state();

    return retgeo;
  }

  Geometry *multipolygon_intersection_multipolygon(Geometry *g1, Geometry *g2,
                                                   String *result) {
    Geometry *retgeo = nullptr;

    const void *pg1 = g1->normalize_ring_order();
    const void *pg2 = g2->normalize_ring_order();
    if (pg1 == nullptr || pg2 == nullptr) {
      null_value = true;
      my_error(ER_GIS_INVALID_DATA, MYF(0), "st_intersection");
      return nullptr;
    }

    Multilinestring mls;
    Multipolygon mplgn1(pg1, g1->get_data_size(), g1->get_flags(),
                        g1->get_srid());

    std::unique_ptr<Multipolygon> mplgn_result(new Multipolygon());
    mplgn_result->set_srid(g1->get_srid());

    Multipolygon mplgn2(pg2, g2->get_data_size(), g2->get_flags(),
                        g2->get_srid());
    bg::intersection(mplgn1, mplgn2, *mplgn_result);
    plgn_intersection_plgn_mls(mplgn1, mplgn2, *mplgn_result, mls);
    retgeo = combine_mls_mplgn_results(&mls, mplgn_result.get(), result);
    if (retgeo == mplgn_result.get()) mplgn_result.release();

    copy_ifso_state();

    return retgeo;
  }

  Geometry *point_union_point(Geometry *g1, Geometry *g2, String *result) {
    Geometry *retgeo = nullptr;
    Geometry::wkbType gt2 = g2->get_type();
    Point_set ptset;  // Use set to make Points unique.

    Point pt1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
              g1->get_srid());
    Multipoint *mpts = new Multipoint();
    std::unique_ptr<Multipoint> guard(mpts);

    mpts->set_srid(g1->get_srid());
    ptset.insert(pt1);
    if (gt2 == Geometry::wkb_point) {
      Point pt2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                g2->get_srid());
      ptset.insert(pt2);
    } else {
      Multipoint mpts2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());
      ptset.insert(mpts2.begin(), mpts2.end());
    }

    std::copy(ptset.begin(), ptset.end(), std::back_inserter(*mpts));
    if (mpts->size() > 0) {
      retgeo = mpts;
      null_value = m_ifso->assign_result(mpts, result);
      guard.release();
    } else {
      if (!null_value) {
        retgeo = m_ifso->empty_result(result, g1->get_srid());
        copy_ifso_state();
      }
    }
    return retgeo;
  }

  Geometry *point_union_geometry(Geometry *g1, Geometry *g2, String *result) {
    Geometry *retgeo = nullptr;
#if !defined(DBUG_OFF)
    Geometry::wkbType gt2 = g2->get_type();
#endif
    DBUG_ASSERT(gt2 == Geometry::wkb_linestring ||
                gt2 == Geometry::wkb_polygon ||
                gt2 == Geometry::wkb_multilinestring ||
                gt2 == Geometry::wkb_multipolygon);
    if (Ifsr::bg_geo_relation_check(g1, g2, Ifsr::SP_DISJOINT_FUNC,
                                    &null_value) &&
        !null_value) {
      Gis_geometry_collection *geocol = new Gis_geometry_collection(g2, result);
      null_value = (geocol == nullptr || geocol->append_geometry(g1, result));
      retgeo = geocol;
    } else if (null_value) {
      retgeo = nullptr;
      return nullptr;
    } else {
      retgeo = g2;
      null_value = retgeo->as_geometry(result, true);
    }

    return retgeo;
  }

  Geometry *linestring_union_linestring(Geometry *g1, Geometry *g2,
                                        String *result) {
    Linestring ls1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                   g1->get_srid());
    Linestring ls2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                   g2->get_srid());
    std::unique_ptr<Multilinestring> res(new Multilinestring());
    res->set_srid(g1->get_srid());

    boost::geometry::union_(ls1, ls2, *res);
    DBUG_ASSERT(res.get() != nullptr);
    DBUG_ASSERT(res->size() != 0);
    if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result)) {
      my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    return res.release();
  }

  Geometry *linestring_union_polygon(Geometry *g1, Geometry *g2,
                                     String *result) {
    Geometry *retgeo = nullptr;
    const void *g2_wkb = g2->normalize_ring_order();
    if (g2_wkb == nullptr) {
      // Invalid polygon
      my_error(ER_GIS_INVALID_DATA, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    Linestring ls1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                   g1->get_srid());
    Polygon py2(g2_wkb, g2->get_data_size(), g2->get_flags(), g2->get_srid());
    std::unique_ptr<Multilinestring> linestrings(new Multilinestring());
    linestrings->set_srid(g1->get_srid());

    // Union(LineString, Polygon) isn't supported by BG, but it's
    // equivalent to GeometryCollection(Polygon, Difference(LineString,
    // Polygon)).
    boost::geometry::difference(ls1, py2, *linestrings);
    DBUG_ASSERT(linestrings.get() != nullptr);
    if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *linestrings, nullptr) &&
        linestrings->size() > 0) {
      my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    // Return the simplest result possible.
    if (linestrings->size() == 0) {
      // Polygon result.
      g2->as_geometry(result, true);
      retgeo = g2;
    } else {
      // GeometryCollection result containing one Polygon and one or
      // more LineStrings.
      Gis_geometry_collection *collection = new Gis_geometry_collection();
      py2.to_wkb_unparsed();
      collection->append_geometry(&py2, result);
      if (linestrings->size() > 1) {
        collection->append_geometry(&(*linestrings), result);
      } else {
        collection->append_geometry(&(*linestrings)[0], result);
      }
      collection->set_ownmem(false);
      retgeo = collection;
    }

    return retgeo;
  }

  Geometry *linestring_union_multilinestring(Geometry *g1, Geometry *g2,
                                             String *result) {
    Linestring ls1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                   g1->get_srid());
    Multilinestring mls2(g2->get_data_ptr(), g2->get_data_size(),
                         g2->get_flags(), g2->get_srid());
    std::unique_ptr<Multilinestring> res(new Multilinestring);
    res->set_srid(g1->get_srid());

    boost::geometry::union_(ls1, mls2, *res);
    DBUG_ASSERT(res.get() != nullptr);
    DBUG_ASSERT(res->size() != 0);
    if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result)) {
      my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    return res.release();
  }

  Geometry *linestring_union_multipolygon(Geometry *g1, Geometry *g2,
                                          String *result) {
    Geometry *retgeo = nullptr;
    const void *g2_wkb = g2->normalize_ring_order();
    if (g2_wkb == nullptr) {
      // Invalid polygon
      my_error(ER_GIS_INVALID_DATA, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    Linestring ls1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                   g1->get_srid());
    Multipolygon mpy2(g2_wkb, g2->get_data_size(), g2->get_flags(),
                      g2->get_srid());
    std::unique_ptr<Multilinestring> linestrings(new Multilinestring());
    linestrings->set_srid(g1->get_srid());

    // Union(LineString, MultiPolygon) isn't supported by BG, but it's
    // equivalent to GeometryCollection(MultiPolygon,
    // Difference(LineString, MultiPolygon)).
    boost::geometry::difference(ls1, mpy2, *linestrings);
    DBUG_ASSERT(linestrings.get() != nullptr);
    if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *linestrings, nullptr) &&
        linestrings->size() > 0) {
      my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    // Return the simplest result possible.
    if (linestrings->size() == 0) {
      // MultiPolygon result.
      g2->as_geometry(result, true);
      retgeo = g2;
    } else {
      // GeometryCollection result containing one or more Polygons and
      // one or more LineStrings.
      Gis_geometry_collection *collection = new Gis_geometry_collection();
      collection->set_srid(g1->get_srid());

      if (mpy2.size() > 1) {
        collection->append_geometry(&mpy2, result);
      } else {
        mpy2[0].to_wkb_unparsed();
        mpy2[0].set_srid(g1->get_srid());
        collection->append_geometry(&mpy2[0], result);
      }

      if (linestrings->size() > 1) {
        collection->append_geometry(&(*linestrings), result);
      } else {
        (*linestrings)[0].set_srid(g1->get_srid());
        collection->append_geometry(&(*linestrings)[0], result);
      }

      collection->set_ownmem(false);
      retgeo = collection;
    }

    return retgeo;
  }

  Geometry *polygon_union_multilinestring(Geometry *g1, Geometry *g2,
                                          String *result) {
    Geometry *retgeo = nullptr;
    const void *g1_wkb = g1->normalize_ring_order();
    if (g1_wkb == nullptr) {
      // Invalid polygon
      my_error(ER_GIS_INVALID_DATA, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    Polygon py1(g1_wkb, g1->get_data_size(), g1->get_flags(), g1->get_srid());
    Multilinestring mls2(g2->get_data_ptr(), g2->get_data_size(),
                         g2->get_flags(), g2->get_srid());
    std::unique_ptr<Multilinestring> linestrings(new Multilinestring());
    linestrings->set_srid(g1->get_srid());

    // Union(Polygon, MultiLineString) isn't supported by BG, but it's
    // equivalent to GeometryCollection(Polygon,
    // Difference(MultiLineString, Polygon)).
    boost::geometry::difference(mls2, py1, *linestrings);
    DBUG_ASSERT(linestrings.get() != nullptr);
    if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *linestrings, nullptr) &&
        linestrings->size() > 0) {
      my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    // Return the simplest result possible.
    if (linestrings->size() == 0) {
      // Polygon result.
      g2->as_geometry(result, true);
      retgeo = g2;
    } else {
      // GeometryCollection result containing one Polygon and one or
      // more LineStrings.
      Gis_geometry_collection *collection = new Gis_geometry_collection();
      py1.to_wkb_unparsed();
      collection->append_geometry(&py1, result);
      if (linestrings->size() > 1) {
        collection->append_geometry(&(*linestrings), result);
      } else {
        collection->append_geometry(&(*linestrings)[0], result);
      }
      collection->set_ownmem(false);
      retgeo = collection;
    }

    return retgeo;
  }

  Geometry *multipoint_union_multipoint(Geometry *g1, Geometry *g2,
                                        String *result) {
    Geometry *retgeo = nullptr;
    Point_set ptset;
    Multipoint *mpts = new Multipoint();
    std::unique_ptr<Multipoint> guard(mpts);

    mpts->set_srid(g1->get_srid());
    Multipoint mpts1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                     g1->get_srid());
    Multipoint mpts2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                     g2->get_srid());

    ptset.insert(mpts1.begin(), mpts1.end());
    ptset.insert(mpts2.begin(), mpts2.end());
    std::copy(ptset.begin(), ptset.end(), std::back_inserter(*mpts));

    if (mpts->size() > 0) {
      retgeo = mpts;
      null_value = m_ifso->assign_result(mpts, result);
      guard.release();
    } else {
      if (!null_value) {
        retgeo = m_ifso->empty_result(result, g1->get_srid());
        copy_ifso_state();
      }
    }
    return retgeo;
  }

  Geometry *multipoint_union_geometry(Geometry *g1, Geometry *g2,
                                      String *result) {
    Geometry *retgeo = nullptr;
#if !defined(DBUG_OFF)
    Geometry::wkbType gt2 = g2->get_type();
#endif
    Point_set ptset;
    Multipoint mpts(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                    g1->get_srid());

    DBUG_ASSERT(gt2 == Geometry::wkb_linestring ||
                gt2 == Geometry::wkb_polygon ||
                gt2 == Geometry::wkb_multilinestring ||
                gt2 == Geometry::wkb_multipolygon);
    ptset.insert(mpts.begin(), mpts.end());

    Gis_geometry_collection *geocol = new Gis_geometry_collection(g2, result);
    std::unique_ptr<Gis_geometry_collection> guard(geocol);
    bool added = false;

    for (typename Point_set::iterator i = ptset.begin(); i != ptset.end();
         ++i) {
      Point &pt = const_cast<Point &>(*i);
      if (Ifsr::bg_geo_relation_check(&pt, g2, Ifsr::SP_DISJOINT_FUNC,
                                      &null_value)) {
        if (null_value || (null_value = geocol->append_geometry(&pt, result)))
          break;
        added = true;
      }
    }

    if (null_value) return nullptr;

    if (added) {
      // Result is already filled above.
      retgeo = geocol;
      guard.release();
    } else {
      retgeo = g2;
      null_value = g2->as_geometry(result, true);
    }

    return retgeo;
  }

  Geometry *multilinestring_union_multilinestring(Geometry *g1, Geometry *g2,
                                                  String *result) {
    Multilinestring mls1(g1->get_data_ptr(), g1->get_data_size(),
                         g1->get_flags(), g1->get_srid());
    Multilinestring mls2(g2->get_data_ptr(), g2->get_data_size(),
                         g2->get_flags(), g2->get_srid());
    std::unique_ptr<Multilinestring> res(new Multilinestring());
    res->set_srid(g1->get_srid());

    boost::geometry::union_(mls1, mls2, *res);
    DBUG_ASSERT(res.get() != nullptr);
    DBUG_ASSERT(res->size() != 0);
    if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result)) {
      my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    return res.release();
  }

  Geometry *multilinestring_union_multipolygon(Geometry *g1, Geometry *g2,
                                               String *result) {
    Geometry *retgeo = nullptr;
    const void *g2_wkb = g2->normalize_ring_order();
    if (g2_wkb == nullptr) {
      // Invalid polygon
      my_error(ER_GIS_INVALID_DATA, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    Multilinestring mls1(g1->get_data_ptr(), g1->get_data_size(),
                         g1->get_flags(), g1->get_srid());
    Multipolygon mpy2(g2_wkb, g2->get_data_size(), g2->get_flags(),
                      g2->get_srid());
    std::unique_ptr<Multilinestring> linestrings(new Multilinestring());
    linestrings->set_srid(g1->get_srid());

    // Union(MultiLineString, MultiPolygon) isn't supported by BG, but
    // it's equivalent to GeometryCollection(MultiPolygon,
    // Difference(MultiLineString, MultiPolygon)).
    boost::geometry::difference(mls1, mpy2, *linestrings);
    DBUG_ASSERT(linestrings.get() != nullptr);
    if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *linestrings, nullptr) &&
        linestrings->size() > 0) {
      my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    // Return the simplest result possible.
    if (linestrings->size() == 0) {
      // MultiPolygon result.
      g2->as_geometry(result, true);
      retgeo = g2;
    } else {
      // GeometryCollection result containing one or more Polygons and
      // one or more LineStrings.
      Gis_geometry_collection *collection = new Gis_geometry_collection();
      collection->set_srid(g1->get_srid());

      if (mpy2.size() > 1) {
        collection->append_geometry(&mpy2, result);
      } else {
        mpy2[0].to_wkb_unparsed();
        mpy2[0].set_srid(g1->get_srid());
        collection->append_geometry(&mpy2[0], result);
      }

      if (linestrings->size() > 1) {
        collection->append_geometry(&(*linestrings), result);
      } else {
        (*linestrings)[0].set_srid(g1->get_srid());
        collection->append_geometry(&(*linestrings)[0], result);
      }

      collection->set_ownmem(false);
      retgeo = collection;
    }

    return retgeo;
  }

  Geometry *polygon_union_polygon(Geometry *g1, Geometry *g2, String *result) {
    Geometry *retgeo = nullptr;
    const void *g1_wkb = g1->normalize_ring_order();
    const void *g2_wkb = g2->normalize_ring_order();
    if (g1_wkb == nullptr || g2_wkb == nullptr) {
      // Invalid polygon
      my_error(ER_GIS_INVALID_DATA, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    Polygon py1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                g1->get_srid());
    Polygon py2(g2_wkb, g2->get_data_size(), g2->get_flags(), g2->get_srid());
    std::unique_ptr<Multipolygon> res(new Multipolygon());
    res->set_srid(g1->get_srid());

    boost::geometry::union_(py1, py2, *res);
    if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result) &&
        res->size() > 0) {
      my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    if (res->size() == 0) {
      // Invalid polygon
      my_error(ER_GIS_INVALID_DATA, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    retgeo = res.release();
    return retgeo;
  }

  Geometry *polygon_union_multipolygon(Geometry *g1, Geometry *g2,
                                       String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multipolygon, retgeo, union_, Polygon, g1, Multipolygon, g2,
             result, null_value);
    return retgeo;
  }

  Geometry *multipolygon_union_multipolygon(Geometry *g1, Geometry *g2,
                                            String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multipolygon, retgeo, union_, Multipolygon, g1, Multipolygon, g2,
             result, null_value);

    return retgeo;
  }

  Geometry *point_difference_geometry(Geometry *g1, Geometry *g2,
                                      String *result) {
    Geometry *retgeo = nullptr;
    bool is_out = Ifsr::bg_geo_relation_check(g1, g2, Ifsr::SP_DISJOINT_FUNC,
                                              &null_value);

    if (!null_value) {
      if (is_out) {
        retgeo = g1;
        null_value = retgeo->as_geometry(result, true);
      } else {
        retgeo = m_ifso->empty_result(result, g1->get_srid());
        copy_ifso_state();
      }
    }
    return retgeo;
  }

  Geometry *multipoint_difference_geometry(Geometry *g1, Geometry *g2,
                                           String *result) {
    Geometry *retgeo = nullptr;
    Multipoint *mpts = new Multipoint();
    std::unique_ptr<Multipoint> guard(mpts);

    mpts->set_srid(g1->get_srid());
    Multipoint mpts1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                     g1->get_srid());
    Point_set ptset;

    for (typename Multipoint::iterator i = mpts1.begin(); i != mpts1.end();
         ++i) {
      if (Ifsr::bg_geo_relation_check(&(*i), g2, Ifsr::SP_DISJOINT_FUNC,
                                      &null_value)) {
        if (null_value) return nullptr;
        ptset.insert(*i);
      }
    }

    if (ptset.empty() == false) {
      std::copy(ptset.begin(), ptset.end(), std::back_inserter(*mpts));
      null_value = m_ifso->assign_result(mpts, result);
      retgeo = mpts;
      guard.release();
    } else {
      if (!null_value) {
        retgeo = m_ifso->empty_result(result, g1->get_srid());
        copy_ifso_state();
      }
    }
    return retgeo;
  }

  Geometry *linestring_difference_linestring(Geometry *g1, Geometry *g2,
                                             String *result) {
    Geometry *retgeo = nullptr;
    Linestring ls1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                   g1->get_srid());
    Linestring ls2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                   g2->get_srid());
    std::unique_ptr<Multilinestring> res(new Multilinestring());
    res->set_srid(g1->get_srid());

    boost::geometry::difference(ls1, ls2, *res);

    // The call to ls->set_ptr() below assumes that result->length()
    // is the length of the LineString, which is only true if result
    // is empty to begin with.
    DBUG_ASSERT(result->length() == 0);

    if (res->size() == 0) {
      post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result);
      retgeo = m_ifso->empty_result(result, g1->get_srid());
    } else if (res->size() == 1) {
      if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, nullptr)) {
        my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
        null_value = true;
        return nullptr;
      }
      Linestring *ls = new Linestring();
      res->begin()->as_geometry(result, false);
      ls->set_ptr(result->ptr() + GEOM_HEADER_SIZE,
                  result->length() - GEOM_HEADER_SIZE);
      ls->set_ownmem(false);
      retgeo = ls;
    } else {
      if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result)) {
        my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
        null_value = true;
        return nullptr;
      }
      retgeo = res.release();
    }

    return retgeo;
  }

  Geometry *linestring_difference_polygon(Geometry *g1, Geometry *g2,
                                          String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multilinestring, retgeo, difference, Linestring, g1, Polygon, g2,
             result, null_value);

    if (!retgeo && !null_value) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }

    return retgeo;
  }

  Geometry *linestring_difference_multilinestring(Geometry *g1, Geometry *g2,
                                                  String *result) {
    Geometry *retgeo = nullptr;
    Linestring ls1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                   g1->get_srid());
    Multilinestring mls2(g2->get_data_ptr(), g2->get_data_size(),
                         g2->get_flags(), g2->get_srid());
    std::unique_ptr<Multilinestring> res(new Multilinestring());
    res->set_srid(g1->get_srid());

    boost::geometry::difference(ls1, mls2, *res);

    // The call to ls->set_ptr() below assumes that result->length()
    // is the length of the LineString, which is only true if result
    // is empty to begin with.
    DBUG_ASSERT(result->length() == 0);

    if (res->size() == 0) {
      post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result);
      retgeo = m_ifso->empty_result(result, g1->get_srid());
    } else if (res->size() == 1) {
      if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, nullptr)) {
        my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
        null_value = true;
        return nullptr;
      }
      Linestring *ls = new Linestring();
      res->begin()->as_geometry(result, false);
      ls->set_ptr(result->ptr() + GEOM_HEADER_SIZE,
                  result->length() - GEOM_HEADER_SIZE);
      ls->set_ownmem(false);
      retgeo = ls;
    } else {
      if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result)) {
        my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
        null_value = true;
        return nullptr;
      }
      retgeo = res.release();
    }

    return retgeo;
  }

  Geometry *linestring_difference_multipolygon(Geometry *g1, Geometry *g2,
                                               String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multilinestring, retgeo, difference, Linestring, g1, Multipolygon,
             g2, result, null_value);

    if (!retgeo && !null_value) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }

  Geometry *polygon_difference_polygon(Geometry *g1, Geometry *g2,
                                       String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multipolygon, retgeo, difference, Polygon, g1, Polygon, g2, result,
             null_value);

    if (!retgeo && !null_value) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }

  Geometry *polygon_difference_multipolygon(Geometry *g1, Geometry *g2,
                                            String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multipolygon, retgeo, difference, Polygon, g1, Multipolygon, g2,
             result, null_value);

    if (!retgeo && !null_value) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }

  Geometry *multilinestring_difference_linestring(Geometry *g1, Geometry *g2,
                                                  String *result) {
    Geometry *retgeo = nullptr;
    Multilinestring mls1(g1->get_data_ptr(), g1->get_data_size(),
                         g1->get_flags(), g1->get_srid());
    Linestring ls2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                   g2->get_srid());
    std::unique_ptr<Multilinestring> res(new Multilinestring());
    res->set_srid(g1->get_srid());

    boost::geometry::difference(mls1, ls2, *res);

    // The call to ls->set_ptr() below assumes that result->length()
    // is the length of the LineString, which is only true if result
    // is empty to begin with.
    DBUG_ASSERT(result->length() == 0);

    if (res->size() == 0) {
      post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result);
      retgeo = m_ifso->empty_result(result, g1->get_srid());
    } else if (res->size() == 1) {
      if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, nullptr)) {
        my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
        null_value = true;
        return nullptr;
      }
      Linestring *ls = new Linestring();
      res->begin()->as_geometry(result, false);
      ls->set_ptr(result->ptr() + GEOM_HEADER_SIZE,
                  result->length() - GEOM_HEADER_SIZE);
      ls->set_ownmem(false);
      retgeo = ls;
    } else {
      if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result)) {
        my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
        null_value = true;
        return nullptr;
      }
      retgeo = res.release();
    }

    return retgeo;
  }

  Geometry *multilinestring_difference_polygon(Geometry *g1, Geometry *g2,
                                               String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multilinestring, retgeo, difference, Multilinestring, g1, Polygon,
             g2, result, null_value);

    if (!retgeo && !null_value) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }

  Geometry *multilinestring_difference_multilinestring(Geometry *g1,
                                                       Geometry *g2,
                                                       String *result) {
    Geometry *retgeo = nullptr;
    Multilinestring mls1(g1->get_data_ptr(), g1->get_data_size(),
                         g1->get_flags(), g1->get_srid());
    Multilinestring mls2(g2->get_data_ptr(), g2->get_data_size(),
                         g2->get_flags(), g2->get_srid());
    std::unique_ptr<Multilinestring> res(new Multilinestring());
    res->set_srid(g1->get_srid());

    boost::geometry::difference(mls1, mls2, *res);

    // The call to ls->set_ptr() below assumes that result->length()
    // is the length of the LineString, which is only true if result
    // is empty to begin with.
    DBUG_ASSERT(result->length() == 0);

    if (res->size() == 0) {
      post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result);
      retgeo = m_ifso->empty_result(result, g1->get_srid());
    } else if (res->size() == 1) {
      if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, nullptr)) {
        my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
        null_value = true;
        return nullptr;
      }
      Linestring *ls = new Linestring();
      res->begin()->as_geometry(result, false);
      ls->set_ptr(result->ptr() + GEOM_HEADER_SIZE,
                  result->length() - GEOM_HEADER_SIZE);
      ls->set_ownmem(false);
      retgeo = ls;
    } else {
      if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result)) {
        my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
        null_value = true;
        return nullptr;
      }
      retgeo = res.release();
    }

    return retgeo;
  }

  Geometry *multilinestring_difference_multipolygon(Geometry *g1, Geometry *g2,
                                                    String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multilinestring, retgeo, difference, Multilinestring, g1,
             Multipolygon, g2, result, null_value);

    if (!retgeo && !null_value) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }

  Geometry *multipolygon_difference_polygon(Geometry *g1, Geometry *g2,
                                            String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multipolygon, retgeo, difference, Multipolygon, g1, Polygon, g2,
             result, null_value);

    if (!retgeo && !null_value) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }

  Geometry *multipolygon_difference_multipolygon(Geometry *g1, Geometry *g2,
                                                 String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multipolygon, retgeo, difference, Multipolygon, g1, Multipolygon,
             g2, result, null_value);

    if (!retgeo && !null_value) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }

  Geometry *linestring_symdifference_linestring(Geometry *g1, Geometry *g2,
                                                String *result) {
    Geometry *retgeo = nullptr;
    Linestring ls1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                   g1->get_srid());
    Linestring ls2(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                   g2->get_srid());
    std::unique_ptr<Multilinestring> res(new Multilinestring());
    res->set_srid(g1->get_srid());

    boost::geometry::sym_difference(ls1, ls2, *res);
    DBUG_ASSERT(res.get() != nullptr);
    if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result) &&
        res->size() > 0) {
      my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    if (res->size() == 0) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
    } else {
      retgeo = res.release();
    }

    return retgeo;
  }

  Geometry *linestring_symdifference_multilinestring(Geometry *g1, Geometry *g2,
                                                     String *result) {
    Geometry *retgeo = nullptr;
    Linestring ls1(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                   g1->get_srid());
    Multilinestring mls2(g2->get_data_ptr(), g2->get_data_size(),
                         g2->get_flags(), g2->get_srid());
    std::unique_ptr<Multilinestring> res(new Multilinestring());
    res->set_srid(g1->get_srid());

    boost::geometry::sym_difference(ls1, mls2, *res);
    DBUG_ASSERT(res.get() != nullptr);
    if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result) &&
        res->size() > 0) {
      my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    if (res->size() == 0) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
    } else {
      retgeo = res.release();
    }

    return retgeo;
  }

  Geometry *polygon_symdifference_polygon(Geometry *g1, Geometry *g2,
                                          String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multipolygon, retgeo, sym_difference, Polygon, g1, Polygon, g2,
             result, null_value);

    if (!retgeo && !null_value) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }

  Geometry *polygon_symdifference_multipolygon(Geometry *g1, Geometry *g2,
                                               String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multipolygon, retgeo, sym_difference, Polygon, g1, Multipolygon,
             g2, result, null_value);

    if (!retgeo && !null_value) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }

  Geometry *multilinestring_symdifference_multilinestring(Geometry *g1,
                                                          Geometry *g2,
                                                          String *result) {
    Geometry *retgeo = nullptr;
    Multilinestring mls1(g1->get_data_ptr(), g1->get_data_size(),
                         g1->get_flags(), g1->get_srid());
    Multilinestring mls2(g2->get_data_ptr(), g2->get_data_size(),
                         g2->get_flags(), g2->get_srid());
    std::unique_ptr<Multilinestring> res(new Multilinestring());
    res->set_srid(g1->get_srid());

    boost::geometry::sym_difference(mls1, mls2, *res);
    DBUG_ASSERT(res.get() != nullptr);
    if (post_fix_result(&m_ifso->m_bg_resbuf_mgr, *res, result) &&
        res->size() > 0) {
      my_error(ER_GIS_UNKNOWN_ERROR, MYF(0), m_ifso->func_name());
      null_value = true;
      return nullptr;
    }

    if (res->size() == 0) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
    } else {
      retgeo = res.release();
    }

    return retgeo;
  }

  Geometry *multipolygon_symdifference_polygon(Geometry *g1, Geometry *g2,
                                               String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multipolygon, retgeo, sym_difference, Multipolygon, g1, Polygon,
             g2, result, null_value);

    if (!retgeo && !null_value) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }

  Geometry *multipolygon_symdifference_multipolygon(Geometry *g1, Geometry *g2,
                                                    String *result) {
    Geometry *retgeo = nullptr;

    BGOPCALL(Multipolygon, retgeo, sym_difference, Multipolygon, g1,
             Multipolygon, g2, result, null_value);

    if (!retgeo && !null_value) {
      retgeo = m_ifso->empty_result(result, g1->get_srid());
      copy_ifso_state();
    }
    return retgeo;
  }
};

/**
  Do intersection operation for two geometries, dispatch to specific BG
  function wrapper calls according to set operation type, and the 1st or
  both operand types.

  @tparam Geom_types A wrapper for all geometry types.
  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] result Holds WKB data of the result.
  @return The result geometry whose WKB data is held in result.
 */
template <typename Geom_types>
Geometry *Item_func_spatial_operation::intersection_operation(Geometry *g1,
                                                              Geometry *g2,
                                                              String *result) {
  BG_setop_wrapper<Geom_types> wrap(this);
  Geometry *retgeo = nullptr;
  Geometry::wkbType gt1 = g1->get_type();
  Geometry::wkbType gt2 = g2->get_type();

  switch (gt1) {
    case Geometry::wkb_point:
      switch (gt2) {
        case Geometry::wkb_point:
          retgeo = wrap.point_intersection_point(g1, g2, result);
          break;
        case Geometry::wkb_multipoint:
          retgeo = wrap.point_intersection_multipoint(g1, g2, result);
          break;
        case Geometry::wkb_linestring:
        case Geometry::wkb_polygon:
        case Geometry::wkb_multilinestring:
        case Geometry::wkb_multipolygon:
          retgeo = wrap.point_intersection_geometry(g1, g2, result);
          break;
        default:
          break;
      }

      break;
    case Geometry::wkb_multipoint:
      switch (gt2) {
        case Geometry::wkb_point:
          retgeo = wrap.point_intersection_multipoint(g2, g1, result);
          break;

        case Geometry::wkb_multipoint:
          retgeo = wrap.multipoint_intersection_multipoint(g1, g2, result);
          break;
        case Geometry::wkb_linestring:
        case Geometry::wkb_polygon:
        case Geometry::wkb_multilinestring:
        case Geometry::wkb_multipolygon:
          retgeo = wrap.multipoint_intersection_geometry(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_linestring:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
          retgeo = intersection_operation<Geom_types>(g2, g1, result);
          break;
        case Geometry::wkb_linestring:
          retgeo = wrap.linestring_intersection_linestring(g1, g2, result);
          break;
        case Geometry::wkb_multilinestring:
          retgeo = wrap.linestring_intersection_multilinestring(g1, g2, result);
          break;
        case Geometry::wkb_polygon:
        case Geometry::wkb_multipolygon:
          retgeo = wrap.linestring_intersection_polygon(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_polygon:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
        case Geometry::wkb_linestring:
          retgeo = intersection_operation<Geom_types>(g2, g1, result);
          break;
        case Geometry::wkb_multilinestring:
          retgeo = wrap.polygon_intersection_multilinestring(g1, g2, result);
          break;
        case Geometry::wkb_polygon:
        case Geometry::wkb_multipolygon:
          // Note: for now BG's set operations don't allow returning a
          // Multilinestring, thus this result isn't complete.
          retgeo = wrap.polygon_intersection_polygon(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_multilinestring:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
        case Geometry::wkb_linestring:
        case Geometry::wkb_polygon:
          retgeo = intersection_operation<Geom_types>(g2, g1, result);
          break;
        case Geometry::wkb_multilinestring:
          retgeo =
              wrap.multilinestring_intersection_multilinestring(g1, g2, result);
          break;

        case Geometry::wkb_multipolygon:
          retgeo =
              wrap.multilinestring_intersection_multipolygon(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_multipolygon:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
        case Geometry::wkb_linestring:
        case Geometry::wkb_multilinestring:
        case Geometry::wkb_polygon:
          retgeo = intersection_operation<Geom_types>(g2, g1, result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.multipolygon_intersection_multipolygon(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

  if (!null_value) null_value = wrap.get_null_value();
  return retgeo;
}

/**
  Do union operation for two geometries, dispatch to specific BG
  function wrapper calls according to set operation type, and the 1st or
  both operand types.

  @tparam Geom_types A wrapper for all geometry types.
  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] result Holds WKB data of the result.
  @return The result geometry whose WKB data is held in result.
 */
template <typename Geom_types>
Geometry *Item_func_spatial_operation::union_operation(Geometry *g1,
                                                       Geometry *g2,
                                                       String *result) {
  BG_setop_wrapper<Geom_types> wrap(this);
  Geometry *retgeo = nullptr;
  Geometry::wkbType gt1 = g1->get_type();
  Geometry::wkbType gt2 = g2->get_type();

  // Note that union can't produce empty point set unless given two empty
  // point set arguments.
  switch (gt1) {
    case Geometry::wkb_point:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
          retgeo = wrap.point_union_point(g1, g2, result);
          break;
        case Geometry::wkb_linestring:
        case Geometry::wkb_multilinestring:
        case Geometry::wkb_polygon:
        case Geometry::wkb_multipolygon:
          retgeo = wrap.point_union_geometry(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_multipoint:
      switch (gt2) {
        case Geometry::wkb_point:
          retgeo = wrap.point_union_point(g2, g1, result);
          break;
        case Geometry::wkb_multipoint:
          retgeo = wrap.multipoint_union_multipoint(g1, g2, result);
          break;
        case Geometry::wkb_linestring:
        case Geometry::wkb_multilinestring:
        case Geometry::wkb_polygon:
        case Geometry::wkb_multipolygon:
          retgeo = wrap.multipoint_union_geometry(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_linestring:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
          retgeo = union_operation<Geom_types>(g2, g1, result);
          break;
        case Geometry::wkb_linestring:
          retgeo = wrap.linestring_union_linestring(g1, g2, result);
          break;
        case Geometry::wkb_multilinestring:
          retgeo = wrap.linestring_union_multilinestring(g1, g2, result);
          break;
        case Geometry::wkb_polygon:
          retgeo = wrap.linestring_union_polygon(g1, g2, result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.linestring_union_multipolygon(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_polygon:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
        case Geometry::wkb_linestring:
          retgeo = union_operation<Geom_types>(g2, g1, result);
          break;
        case Geometry::wkb_multilinestring:
          retgeo = wrap.polygon_union_multilinestring(g1, g2, result);
          break;
        case Geometry::wkb_polygon:
          retgeo = wrap.polygon_union_polygon(g1, g2, result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.polygon_union_multipolygon(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_multilinestring:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
        case Geometry::wkb_linestring:
        case Geometry::wkb_polygon:
          retgeo = union_operation<Geom_types>(g2, g1, result);
          break;
          break;
        case Geometry::wkb_multilinestring:
          retgeo = wrap.multilinestring_union_multilinestring(g1, g2, result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.multilinestring_union_multipolygon(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_multipolygon:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
        case Geometry::wkb_linestring:
        case Geometry::wkb_polygon:
        case Geometry::wkb_multilinestring:
          retgeo = union_operation<Geom_types>(g2, g1, result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.multipolygon_union_multipolygon(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

  if (!null_value) null_value = wrap.get_null_value() && maybe_null;
  if (!null_value && retgeo == nullptr) {
    my_error(ER_GIS_INVALID_DATA, MYF(0), func_name());
    error_str();
    return nullptr;
  }
  return retgeo;
}

/**
  Do difference operation for two geometries, dispatch to specific BG
  function wrapper calls according to set operation type, and the 1st or
  both operand types.

  @tparam Geom_types A wrapper for all geometry types.
  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] result Holds WKB data of the result.
  @return The result geometry whose WKB data is held in result.
 */
template <typename Geom_types>
Geometry *Item_func_spatial_operation::difference_operation(Geometry *g1,
                                                            Geometry *g2,
                                                            String *result) {
  BG_setop_wrapper<Geom_types> wrap(this);
  Geometry *retgeo = nullptr;
  Geometry::wkbType gt1 = g1->get_type();
  Geometry::wkbType gt2 = g2->get_type();

  /*
    Given two geometries g1 and g2, where g1.dimension < g2.dimension, then
    g2 - g1 is equal to g2, this is always true. This is how postgis works.
    Below implementation uses this fact.
   */
  switch (gt1) {
    case Geometry::wkb_point:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
        case Geometry::wkb_linestring:
        case Geometry::wkb_polygon:
        case Geometry::wkb_multilinestring:
        case Geometry::wkb_multipolygon:
          retgeo = wrap.point_difference_geometry(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_multipoint:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
        case Geometry::wkb_linestring:
        case Geometry::wkb_polygon:
        case Geometry::wkb_multilinestring:
        case Geometry::wkb_multipolygon:
          retgeo = wrap.multipoint_difference_geometry(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_linestring:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
          retgeo = g1;
          null_value = g1->as_geometry(result, true);
          break;
        case Geometry::wkb_linestring:
          retgeo = wrap.linestring_difference_linestring(g1, g2, result);
          break;
        case Geometry::wkb_polygon:
          retgeo = wrap.linestring_difference_polygon(g1, g2, result);
          break;
        case Geometry::wkb_multilinestring:
          retgeo = wrap.linestring_difference_multilinestring(g1, g2, result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.linestring_difference_multipolygon(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_polygon:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
        case Geometry::wkb_linestring:
        case Geometry::wkb_multilinestring:
          retgeo = g1;
          null_value = g1->as_geometry(result, true);
          break;
        case Geometry::wkb_polygon:
          retgeo = wrap.polygon_difference_polygon(g1, g2, result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.polygon_difference_multipolygon(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_multilinestring:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
          retgeo = g1;
          null_value = g1->as_geometry(result, true);
          break;
        case Geometry::wkb_linestring:
          retgeo = wrap.multilinestring_difference_linestring(g1, g2, result);
          break;
        case Geometry::wkb_polygon:
          retgeo = wrap.multilinestring_difference_polygon(g1, g2, result);
          break;
        case Geometry::wkb_multilinestring:
          retgeo =
              wrap.multilinestring_difference_multilinestring(g1, g2, result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.multilinestring_difference_multipolygon(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    case Geometry::wkb_multipolygon:
      switch (gt2) {
        case Geometry::wkb_point:
        case Geometry::wkb_multipoint:
        case Geometry::wkb_linestring:
        case Geometry::wkb_multilinestring:
          retgeo = g1;
          null_value = g1->as_geometry(result, true);
          break;
        case Geometry::wkb_polygon:
          retgeo = wrap.multipolygon_difference_polygon(g1, g2, result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.multipolygon_difference_multipolygon(g1, g2, result);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

  if (!null_value) null_value = wrap.get_null_value();
  return retgeo;
}

/**
  Do symdifference operation for two geometries, dispatch to specific BG
  function wrapper calls according to set operation type, and the 1st or
  both operand types.

  @tparam Geom_types A wrapper for all geometry types.
  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] result Holds WKB data of the result.
  @return The result geometry whose WKB data is held in result.
 */
template <typename Geom_types>
Geometry *Item_func_spatial_operation::symdifference_operation(Geometry *g1,
                                                               Geometry *g2,
                                                               String *result) {
  typedef typename Geom_types::Coordinate_system Coordsys;

  BG_setop_wrapper<Geom_types> wrap(this);
  Geometry *retgeo = nullptr;
  Geometry::wkbType gt1 = g1->get_type();
  Geometry::wkbType gt2 = g2->get_type();

  /*
    SymDifference(L, A) isn't supported by BG, but it's equivalent
    to

        GeometryCollection(Difference(A, L), Difference(L, A))

    Since geometries must be closed, Difference(A, L) is equivalent
    to A, so we can simplify to

        GeometryCollection(A, Difference(L, A))

    This is equivalent to Union(L, A), so we reuse that implementation.
   */
  bool do_geocol_setop = false;

  switch (gt1) {
    case Geometry::wkb_linestring:
      switch (gt2) {
        case Geometry::wkb_linestring:
          retgeo = wrap.linestring_symdifference_linestring(g1, g2, result);
          break;
        case Geometry::wkb_polygon:
          retgeo = wrap.linestring_union_polygon(g1, g2, result);
          break;
        case Geometry::wkb_multilinestring:
          retgeo =
              wrap.linestring_symdifference_multilinestring(g1, g2, result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.linestring_union_multipolygon(g1, g2, result);
          break;
        default:
          do_geocol_setop = true;
          break;
      }
      break;
    case Geometry::wkb_polygon:

      switch (gt2) {
        case Geometry::wkb_linestring:
          retgeo = wrap.linestring_union_polygon(g2, g1, result);
          break;
        case Geometry::wkb_polygon:
          retgeo = wrap.polygon_symdifference_polygon(g1, g2, result);
          break;
        case Geometry::wkb_multilinestring:
          retgeo = wrap.polygon_union_multilinestring(g1, g2, result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.polygon_symdifference_multipolygon(g1, g2, result);
          break;
        default:
          do_geocol_setop = true;
          break;
      }
      break;
    case Geometry::wkb_multilinestring:
      switch (gt2) {
        case Geometry::wkb_linestring:
          retgeo =
              wrap.linestring_symdifference_multilinestring(g2, g1, result);
          break;
        case Geometry::wkb_polygon:
          retgeo = wrap.polygon_union_multilinestring(g2, g1, result);
          break;
        case Geometry::wkb_multilinestring:
          retgeo = wrap.multilinestring_symdifference_multilinestring(g1, g2,
                                                                      result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.multilinestring_union_multipolygon(g1, g2, result);
          break;
        default:
          do_geocol_setop = true;
          break;
      }
      break;
    case Geometry::wkb_multipolygon:
      switch (gt2) {
        case Geometry::wkb_linestring:
          retgeo = wrap.linestring_union_multipolygon(g2, g1, result);
          break;
        case Geometry::wkb_polygon:
          retgeo = wrap.multipolygon_symdifference_polygon(g1, g2, result);
          break;
        case Geometry::wkb_multilinestring:
          retgeo = wrap.multilinestring_union_multipolygon(g2, g1, result);
          break;
        case Geometry::wkb_multipolygon:
          retgeo = wrap.multipolygon_symdifference_multipolygon(g1, g2, result);
          break;
        default:
          do_geocol_setop = true;
          break;
      }
      break;
    default:
      do_geocol_setop = true;
      break;
  }

  if (do_geocol_setop)
    retgeo = geometry_collection_set_operation<Coordsys>(g1, g2, result);
  else if (!null_value)
    null_value = wrap.get_null_value();
  return retgeo;
}

/**
  Call boost geometry set operations to compute set operation result, and
  returns the result as a Geometry object.

  @tparam Coordsys Coordinate system type, specified using those defined in
          boost::geometry::cs.
  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] result buffer containing the GEOMETRY byte string of
  the returned geometry.

  @return If the set operation results in an empty point set, return a
  geometry collection containing 0 objects. If null_value is set to
  true, always returns 0. The returned geometry object can be used in the same
  val_str call.
 */
template <typename Coordsys>
Geometry *Item_func_spatial_operation::bg_geo_set_op(Geometry *g1, Geometry *g2,
                                                     String *result) {
  typedef BG_models<Coordsys> Geom_types;

  Geometry *retgeo = nullptr;

  if (g1->get_coordsys() != g2->get_coordsys()) return nullptr;

  switch (m_spatial_op) {
    case op_intersection:
      retgeo = intersection_operation<Geom_types>(g1, g2, result);
      break;
    case op_union:
      retgeo = union_operation<Geom_types>(g1, g2, result);
      break;
    case op_difference:
      retgeo = difference_operation<Geom_types>(g1, g2, result);
      break;
    case op_symdifference:
      retgeo = symdifference_operation<Geom_types>(g1, g2, result);
      break;
    default:
      // Other operations are not set operations.
      DBUG_ASSERT(false);
      break;
  }

  /*
    null_value is set in above xxx_operatoin calls if error occurred.
  */
  if (null_value) {
    error_str();
    DBUG_ASSERT(retgeo == nullptr);
  }

  // If we got effective result, the wkb encoding is written to 'result', and
  // the retgeo is effective geometry object whose data points into
  // 'result''s data.
  return retgeo;
}

/**
  Combine sub-results of set operation into a geometry collection.
  This function eliminates points in geo2 that are within
  geo1(polygons or linestrings). We have to do so
  because BG set operations return results in 3 forms --- multipolygon,
  multilinestring and multipoint, however given a type of set operation and
  the operands, the returned 3 types of results may intersect, and we want to
  eliminate the points already in the polygons/linestrings. And in future we
  also need to remove the linestrings that are already in the polygons, this
  isn't done now because there are no such set operation results to combine.

  @tparam Coordsys Coordinate system type, specified using those defined in
          boost::geometry::cs.
  @param geo1 First operand, a Multipolygon or Multilinestring object
              computed by BG set operation.
  @param geo2 Second operand, a Multipoint object
              computed by BG set operation.
  @param[in] default_srid The SRID to use when returning an empty result.
  @param result Holds result geometry's WKB data in GEOMETRY format.
  @return A geometry combined from geo1 and geo2. Either or both of
  geo1 and geo2 can be NULL, so we may end up with a multipoint,
  a multipolygon/multilinestring, a geometry collection, or an
  empty geometry collection.
 */
template <typename Coordsys>
Geometry *Item_func_spatial_operation::combine_sub_results(
    Geometry *geo1, Geometry *geo2, gis::srid_t default_srid, String *result) {
  typedef BG_models<Coordsys> Geom_types;
  typedef typename Geom_types::Multipoint Multipoint;
  Geometry *retgeo = nullptr;
  bool isin = false, added = false;

  if (null_value) {
    delete geo1;
    delete geo2;
    return nullptr;
  }

  std::unique_ptr<Geometry> guard1(geo1), guard2(geo2);

  Gis_geometry_collection *geocol = nullptr;
  if (geo1 == nullptr && geo2 == nullptr)
    retgeo = empty_result(result, default_srid);
  else if (geo1 != nullptr && geo2 == nullptr) {
    retgeo = geo1;
    null_value = assign_result(geo1, result);
    guard1.release();
  } else if (geo1 == nullptr && geo2 != nullptr) {
    retgeo = geo2;
    null_value = assign_result(geo2, result);
    guard2.release();
  }

  if (geo1 == nullptr || geo2 == nullptr) {
    if (null_value) retgeo = nullptr;
    return retgeo;
  }

  DBUG_ASSERT((geo1->get_type() == Geometry::wkb_multilinestring ||
               geo1->get_type() == Geometry::wkb_multipolygon) &&
              geo2->get_type() == Geometry::wkb_multipoint);
  Multipoint mpts(geo2->get_data_ptr(), geo2->get_data_size(),
                  geo2->get_flags(), geo2->get_srid());
  geocol = new Gis_geometry_collection(geo1, result);
  geocol->set_components_no_overlapped(geo1->is_components_no_overlapped());
  std::unique_ptr<Gis_geometry_collection> guard3(geocol);
  bool had_error = false;

  for (typename Multipoint::iterator i = mpts.begin(); i != mpts.end(); ++i) {
    isin = !Item_func_spatial_rel::bg_geo_relation_check(
        &(*i), geo1, SP_DISJOINT_FUNC, &had_error);

    if (had_error) {
      error_str();
      return nullptr;
    }

    if (!isin) {
      geocol->append_geometry(&(*i), result);
      added = true;
    }
  }

  if (added) {
    retgeo = geocol;
    guard3.release();
  } else {
    retgeo = geo1;
    guard1.release();
    null_value = assign_result(geo1, result);
  }

  if (null_value) error_str();

  return retgeo;
}

/**
  Simplify a multilinestring result from the BG intersection function.

  The multilinestring may contain linestrings with two points where
  both points are the same. Those are intersection points that should
  be returned to the user as points. This function loops through the
  multilinestring, separating true linestrings and linestring-encoded
  points, and returns the simplest geometric result possible: point,
  linestring, multilinestring, multipoint, or geometry collection.

  @param mls The multilinestring to simplify
  @param[out] result The GEOMETRY string (SRID+WKB) of the returned object
  @return The simplest geometry type representing the input.
*/
Geometry *Item_func_spatial_operation::simplify_multilinestring(
    Gis_multi_line_string *mls, String *result) {
  // Null values are handled by caller.
  DBUG_ASSERT(mls != nullptr);
  Geometry *retgeo;

  // Loop through the multilinestring and separate true linestrings
  // from points.
  std::unique_ptr<Gis_multi_line_string> linestrings(
      new Gis_multi_line_string());
  std::unique_ptr<Gis_multi_point> points(new Gis_multi_point());
  linestrings->set_srid(mls->get_srid());
  points->set_srid(mls->get_srid());
  // BG may return duplicate points, so use a point set to get unique
  // points before storing them into 'points'.
  typedef std::set<Gis_point, bgpt_lt> Point_set;
  Point_set point_set(points->begin(), points->end());
  Gis_point prev_point;
  bool has_prev_point = false;

  for (Gis_multi_line_string::iterator i = mls->begin(); i != mls->end(); ++i) {
    i->set_srid(mls->get_srid());
    if (i->size() != 2) {
      DBUG_ASSERT(i->size() > 2);
      linestrings->push_back(*i);
      continue;
    }

    (*i)[0].set_srid(mls->get_srid());
    (*i)[1].set_srid(mls->get_srid());
    const Gis_point &start = (*i)[0];
    const Gis_point &end = (*i)[1];
    if (start == end) {
      if (!has_prev_point || prev_point != start) {
        has_prev_point = true;
        prev_point = start;
        point_set.insert(start);
      }
    } else
      linestrings->push_back(*i);
  }

  for (Point_set::iterator i = point_set.begin(); i != point_set.end(); ++i) {
    /*
      When computing [m]ls intersection [m]ls, BG may return points that are
      on the linestrings, so here we have to exclude such points.
    */
    if (bg::disjoint(*i, *linestrings)) points->push_back(*i);
  }

  String dummy;
  post_fix_result(&m_bg_resbuf_mgr, *linestrings, &dummy);
  post_fix_result(&m_bg_resbuf_mgr, *points, &dummy);

  // Return the simplest type possible
  if (points->size() == 0 && linestrings->size() == 1) {
    // Linestring result
    Gis_line_string *linestringresult = new Gis_line_string();
    linestringresult->set_srid(mls->get_srid());
    size_t oldlength = result->length();
    linestrings->begin()->as_geometry(result, false);
    size_t newlength = result->length();
    linestringresult->set_ptr(result->ptr() + oldlength + GEOM_HEADER_SIZE,
                              (newlength - oldlength) - GEOM_HEADER_SIZE);
    linestringresult->set_ownmem(false);
    retgeo = linestringresult;
  } else if (points->size() == 0 && linestrings->size() > 1) {
    // Multilinestring result
    linestrings->as_geometry(result, false);
    linestrings->set_ownmem(false);
    retgeo = linestrings.release();
  } else if (points->size() == 1 && linestrings->size() == 0) {
    // Point result
    Gis_point *pointresult = new Gis_point();
    pointresult->set_srid(mls->get_srid());
    size_t oldlength = result->length();
    points->begin()->as_geometry(result, false);
    size_t newlength = result->length();
    pointresult->set_ptr(result->ptr() + oldlength + GEOM_HEADER_SIZE,
                         (newlength - oldlength) - GEOM_HEADER_SIZE);
    pointresult->set_ownmem(false);
    retgeo = pointresult;
  } else if (points->size() > 1 && linestrings->size() == 0) {
    // Multipoint result
    points->as_geometry(result, false);
    points->set_ownmem(false);
    retgeo = points.release();
  } else {
    // GeometryCollection result
    Gis_geometry_collection *collection = new Gis_geometry_collection();
    collection->set_srid(mls->get_srid());

    if (points->size() > 1) {
      collection->append_geometry(&(*points), result);
    } else {
      collection->append_geometry(&(*points)[0], result);
    }

    if (linestrings->size() > 1) {
      collection->append_geometry(&(*linestrings), result);
    } else {
      collection->append_geometry(&(*linestrings)[0], result);
    }

    collection->set_ownmem(false);
    retgeo = collection;
  }

  return retgeo;
}

/**
  Extract a basic geometry component from a multi geometry or a geometry
  collection, if it's the only one in it.
 */
class Singleton_extractor : public WKB_scanner_event_handler {
  /*
    If we see the nested geometries as a forest, seeing the outmost one as the
    ground where the trees grow, and seeing each of its components
    as a tree, then the search for a singleton in a geometry collection(GC) or
    multi-geometry(i.e. multipoint, multilinestring, multipolygon) is identical
    to searching on the ground to see if there is only one tree on the ground,
    if so we also need to record its starting address within the root node's
    memory buffer.

    Some details complicate the problem:
    1. GCs can be nested into another GC, a nested GC should be see also as
       the 'ground' rather than a tree.
    2. A single multi-geometry contained in a GC may be a singleton or not.
       a. When it has only one component in itself, that component is
          the singleton;
       b. Otherwise itself is the singleton.
    3. Basic geometries are always atomic(undevidible).
    4. A multi-geometry can't be nested into another multigeometry, it can
       only be a component of a GC.

    Below comment for the data members are based on this context information.
  */
  // The number of trees on the ground.
  int ntrees;
  // The number of trees inside all multi-geometries.
  int nsubtrees;
  // Current tree travasal stack depth, i.e. tree height.
  int depth;
  // The depth of the multi-geometry, if any.
  int mg_depth;
  // The effective stack depth, i.e. excludes the nested GCs.
  int levels;
  // The stack depth of heighest GC in current ground.
  int gc_depth;
  // Starting and ending address of tree on ground.
  const char *start, *end;
  // Starting address of and type of the basic geometry which is on top of the
  // multi-geometry.
  const char *bg_start;
  Geometry::wkbType bg_type;

  // The type of the geometry on the ground.
  Geometry::wkbType gtype;

 public:
  Singleton_extractor() {
    ntrees = nsubtrees = depth = mg_depth = levels = gc_depth = 0;
    bg_start = start = end = nullptr;
    bg_type = gtype = Geometry::wkb_invalid_type;
  }

  static bool is_basic_type(const Geometry::wkbType t) {
    return t == Geometry::wkb_point || t == Geometry::wkb_linestring ||
           t == Geometry::wkb_polygon;
  }

  bool has_single_component() const { return ntrees == 1; }

  // Functions to get singleton information.

  /*
    Returns start of singleton. If only one sub-tree, the basic geometry
    is returned instead of the multi-geometry, otherwise the multi-geometry
    is returned.
   */
  const char *get_start() const { return nsubtrees == 1 ? bg_start : start; }

  /*
    Returns the end of the singleton geometry. For a singleton,
    its end is always also the end of the root geometry, so this function
    is correct only when the root geometry really contains a singleton.
   */
  const char *get_end() const { return end; }

  Geometry::wkbType get_type() const {
    return nsubtrees == 1 ? bg_type : gtype;
  }

  virtual void on_wkb_start(Geometry::wkbByteOrder, Geometry::wkbType geotype,
                            const void *wkb, uint32 len, bool) {
    if (geotype != Geometry::wkb_geometrycollection) {
      if (gc_depth == 0) {
        gc_depth = depth;
        start = static_cast<const char *>(wkb);
        end = start + len;
        gtype = geotype;
      }

      if (!is_basic_type(geotype)) mg_depth = depth;

      if (mg_depth + 1 == depth) {
        bg_type = geotype;
        bg_start = static_cast<const char *>(wkb);
      }

      levels++;
    } else
      gc_depth = 0;

    depth++;
  }

  virtual void on_wkb_end(const void *wkb) {
    depth--;
    DBUG_ASSERT(depth >= 0);

    if (levels > 0) {
      levels--;
      if (levels == 0) {
        DBUG_ASSERT(depth == gc_depth);
        ntrees++;
        end = static_cast<const char *>(wkb);
        mg_depth = 0;
        gc_depth = 0;
      }
    }

    // The subtree is either a multi-geometry or a basic geometry.
    if (mg_depth != 0 && levels == 1) nsubtrees++;
  }
};

inline Geometry::wkbType base_type(Geometry::wkbType gt) {
  Geometry::wkbType ret;

  switch (gt) {
    case Geometry::wkb_multipoint:
      ret = Geometry::wkb_point;
      break;
    case Geometry::wkb_multilinestring:
      ret = Geometry::wkb_linestring;
      break;
    case Geometry::wkb_multipolygon:
      ret = Geometry::wkb_polygon;
      break;
    default:
      ret = gt;
  }
  return ret;
}

/**
  Simplify multi-geometry data. If str contains a multi-geometry or geometry
  collection with one component, the component is made as content of str.
  If str contains a nested geometry collection, the effective concrete geometry
  object is returned.
  @param str A string buffer containing a GEOMETRY byte string.
  @param [out] result_buffer if not NULL and if a simplification is to be done,
               we will copy the GEOMETRY byte string from str into result_buffer
               and simplify the one stored in result_buffer, and the one in str
               is intact. If there is any data in result_buffer before calling
               this function, it is overwritten; If no simplification done, the
               result_buffer is intact if it is provided.
  @return whether the geometry is simplified or not.
 */
bool simplify_multi_geometry(String *str, String *result_buffer) {
  if (str->length() < GEOM_HEADER_SIZE) return false;

  char *p = str->ptr();
  Geometry::wkbType gtype = get_wkb_geotype(p + 5);
  bool ret = false;

  if (gtype == Geometry::wkb_multipoint ||
      gtype == Geometry::wkb_multilinestring ||
      gtype == Geometry::wkb_multipolygon) {
    if (uint4korr(p + GEOM_HEADER_SIZE) == 1) {
      if (result_buffer) {
        result_buffer->length(0);
        result_buffer->append(*str);
        p = result_buffer->ptr();
        str = result_buffer;
      }
      DBUG_ASSERT((str->length() - GEOM_HEADER_SIZE - 4 - WKB_HEADER_SIZE) > 0);
      int4store(p + 5, static_cast<uint32>(base_type(gtype)));
      memmove(p + GEOM_HEADER_SIZE, p + GEOM_HEADER_SIZE + 4 + WKB_HEADER_SIZE,
              str->length() - GEOM_HEADER_SIZE - 4 - WKB_HEADER_SIZE);
      str->length(str->length() - 4 - WKB_HEADER_SIZE);
      ret = true;
    }
  } else if (gtype == Geometry::wkb_geometrycollection) {
    Singleton_extractor ex;
    uint32 wkb_len = str->length() - GEOM_HEADER_SIZE;
    wkb_scanner(current_thd, p + GEOM_HEADER_SIZE, &wkb_len,
                Geometry::wkb_geometrycollection, false, &ex);
    if (ex.has_single_component()) {
      if (result_buffer) {
        result_buffer->length(0);
        result_buffer->append(*str);
        p = result_buffer->ptr();
        str = result_buffer;
      }
      p = write_wkb_header(p + 4, ex.get_type());
      ptrdiff_t len = ex.get_end() - ex.get_start();
      DBUG_ASSERT(len > 0);
      memmove(p, ex.get_start(), len);
      str->length(GEOM_HEADER_SIZE + len);
      ret = true;
    }
  }

  return ret;
}

/*
  Do set operations on geometries.
  Writes geometry set operation result into str_value_arg in wkb format.
 */
String *Item_func_spatial_operation::val_str(String *str_value_arg) {
  DBUG_TRACE;
  DBUG_ASSERT(fixed == 1);

  m_tmp_value1.length(0);
  m_tmp_value2.length(0);
  String *res1 = args[0]->val_str(&m_tmp_value1);
  String *res2 = args[1]->val_str(&m_tmp_value2);
  Geometry_buffer buffer1, buffer2;
  Geometry *g1 = nullptr, *g2 = nullptr, *gres = nullptr;
  bool had_except1 = false, had_except2 = false;
  bool result_is_args = false;

  // Release last call's result buffer.
  m_bg_resbuf_mgr.free_result_buffer();

  if ((null_value =
           (!res1 || args[0]->null_value || !res2 || args[1]->null_value)))
    return nullptr;

  if (!(g1 = Geometry::construct(&buffer1, res1)) ||
      !(g2 = Geometry::construct(&buffer2, res2))) {
    my_error(ER_GIS_INVALID_DATA, MYF(0), func_name());
    return error_str();
  }

  // The two geometry operand must be in the same coordinate system.
  if (g1->get_srid() != g2->get_srid()) {
    my_error(ER_GIS_DIFFERENT_SRIDS, MYF(0), func_name(), g1->get_srid(),
             g2->get_srid());
    return error_str();
  }

  if (g1->get_srid() != 0) {
    THD *thd = current_thd;
    std::unique_ptr<dd::cache::Dictionary_client::Auto_releaser> releaser(
        new dd::cache::Dictionary_client::Auto_releaser(thd->dd_client()));
    Srs_fetcher fetcher(thd);
    const dd::Spatial_reference_system *srs = nullptr;
    if (fetcher.acquire(g1->get_srid(), &srs))
      return error_str();  // Error has already been flagged.

    if (srs == nullptr) {
      my_error(ER_SRS_NOT_FOUND, MYF(0), g1->get_srid());
      return error_str();
    }

    if (!srs->is_cartesian()) {
      DBUG_ASSERT(srs->is_geographic());
      std::string parameters(g1->get_class_info()->m_name.str);
      parameters.append(", ").append(g2->get_class_info()->m_name.str);
      my_error(ER_NOT_IMPLEMENTED_FOR_GEOGRAPHIC_SRS, MYF(0), func_name(),
               parameters.c_str());
      return error_str();
    }
  }

  // Use a local String here, since a BG_result_buf_mgr owns the buffer.
  String str_value;

  /*
    Catch all exceptions to make sure no exception can be thrown out of
    current function. Put all and any code that calls Boost.Geometry functions,
    STL functions into this try block. Code out of the try block should never
    throw any exception.
  */
  try {
    if (g1->get_type() != Geometry::wkb_geometrycollection &&
        g2->get_type() != Geometry::wkb_geometrycollection)
      gres = bg_geo_set_op<bgcs::cartesian>(g1, g2, &str_value);
    else
      gres = geometry_collection_set_operation<bgcs::cartesian>(g1, g2,
                                                                &str_value);

  } catch (...) {
    had_except1 = true;
    handle_gis_exception(func_name());
  }

  try {
    /*
      The buffers in res1 and res2 either belong to argument Item_xxx objects
      or simply belong to m_tmp_value1 or m_tmp_value2, they will be deleted
      properly by their owners, not by our m_bg_resbuf_mgr, so here we must
      forget them in order not to free the buffers before the Item_xxx
      owner nodes are destroyed.
    */
    m_bg_resbuf_mgr.forget_buffer(res1->ptr());
    m_bg_resbuf_mgr.forget_buffer(res2->ptr());
    m_bg_resbuf_mgr.forget_buffer(m_tmp_value1.ptr());
    m_bg_resbuf_mgr.forget_buffer(m_tmp_value2.ptr());

    /*
      Release intermediate geometry data buffers accumulated during execution
      of this set operation.
    */
    if (!str_value.is_alloced() && gres != g1 && gres != g2)
      m_bg_resbuf_mgr.set_result_buffer(str_value.ptr());
    m_bg_resbuf_mgr.free_intermediate_result_buffers();
  } catch (...) {
    had_except2 = true;
    handle_gis_exception(func_name());
  }

  if (had_except1 || had_except2 || null_value) {
    if (gres != nullptr && gres != g1 && gres != g2) {
      delete gres;
      gres = nullptr;
    }
    return error_str();
  }

  DBUG_ASSERT(gres != nullptr && !null_value && str_value.length() > 0);

  /*
    There are 4 ways to create the result geometry object and allocate
    memory for the result String object:
    1. Created in BGOPCALL and allocated by BG code using gis_wkb_alloc
       functions; The geometry result object's memory is took over by
       str_value, thus not allocated by str_value.
    2. Created as a new geometry object and allocated by
       str_value's String member functions.
    3. One of g1 or g2 used as result and g1/g2's String object is used as
       final result without duplicating their byte strings. Also, g1 and/or
       g2 may be used as intermediate result and their byte strings are
       assigned to intermediate String objects without giving the ownerships
       to them, so they are always owned by m_tmp_value1 and/or m_tmp_value2.
    4. A geometry duplicated from a component of BG_geometry_collection.
       when both GCs have 1 member, we do set operation for the two members
       directly, and if such a component is the result we have to duplicate
       it and its WKB String buffer.

    Among above 4 ways, #1, #2 and #4 write the byte string only once without
    any data copying, #3 doesn't write any byte strings.

    And here we always have a GEOMETRY byte string in str_value, although
    in some cases gres->has_geom_header_space() is false.
   */
  if (!str_value.is_alloced() && gres != g1 && gres != g2) {
    DBUG_ASSERT(gres->has_geom_header_space() || gres->is_bg_adapter());
  } else {
    DBUG_ASSERT(gres->has_geom_header_space() || (gres == g1 || gres == g2));
    if (gres == g1) {
      str_value.copy(*res1);
      result_is_args = true;
    } else if (gres == g2) {
      str_value.copy(*res2);
      result_is_args = true;
    }
  }

  /*
    We can not modify the geometry argument because it is assumed to be stable.
    So if returning one of the arguments as result directly, make sure the
    simplification is done in a separate buffer.
  */
  if (simplify_multi_geometry(&str_value,
                              (result_is_args ? &m_result_buffer : nullptr)) &&
      result_is_args)
    str_value.copy(m_result_buffer);

  if (gres != g1 && gres != g2 && gres != nullptr) delete gres;
  // Result and argument SRIDs must be the same.
  DBUG_ASSERT(null_value ||
              uint4korr(str_value.ptr()) == uint4korr(res1->ptr()));

  if (null_value) return nullptr;

  str_value_arg->copy(str_value);
  return str_value_arg;
}

inline bool is_areal(const Geometry *g) {
  return g != nullptr && (g->get_type() == Geometry::wkb_polygon ||
                          g->get_type() == Geometry::wkb_multipolygon);
}

/**
  Do set operation on geometry collections.
  BG doesn't directly support geometry collections in any function, so we
  have to do so by computing the set operation result of all two operands'
  components, which must be the 6 basic types of geometries, and then we
  combine the sub-results.

  This function dispatches to specific set operation types.

  @tparam Coordsys Coordinate system type, specified using those defined in
          boost::geometry::cs.
  @param g1 First geometry operand, a geometry collection.
  @param g2 Second geometry operand, a geometry collection.
  @param[out] result Holds WKB data of the result, which must be a
                geometry collection.
  @return The set operation result, whose WKB data is stored in 'result'.
 */
template <typename Coordsys>
Geometry *Item_func_spatial_operation::geometry_collection_set_operation(
    Geometry *g1, Geometry *g2, String *result) {
  Geometry *gres = nullptr;
  BG_geometry_collection bggc1, bggc2;

  bggc1.set_srid(g1->get_srid());
  bggc2.set_srid(g2->get_srid());
  bool empty1 = is_empty_geocollection(g1);
  bool empty2 = is_empty_geocollection(g2);

  /* Short cut for either one operand being empty. */
  if (empty1 || empty2) {
    if (m_spatial_op == op_intersection ||
        (empty1 && empty2 &&
         (m_spatial_op == op_symdifference || m_spatial_op == op_union)) ||
        (empty1 && m_spatial_op == op_difference)) {
      return empty_result(result, g1->get_srid());
    }

    // If m_spatial_op is op_union or op_symdifference and one of g1/g2 is
    // empty, we will continue in order to merge components in the argument.

    if (empty2 && m_spatial_op == op_difference) {
      null_value = g1->as_geometry(result, true /* shallow copy */);
      return g1;
    }
  }

  bggc1.fill(g1);
  bggc2.fill(g2);
  if (m_spatial_op != op_union) {
    bggc1.merge_components<Coordsys>(&null_value);
    if (null_value) return gres;
    bggc2.merge_components<Coordsys>(&null_value);
    if (null_value) return gres;
  }

  BG_geometry_collection::Geometry_list &gv1 = bggc1.get_geometries();
  BG_geometry_collection::Geometry_list &gv2 = bggc2.get_geometries();

  /*
    If there is only one component in one argument and the other is empty,
    no merge is possible.
  */
  if (m_spatial_op == op_union || m_spatial_op == op_symdifference) {
    if (gv1.size() == 0 && gv2.size() == 1) {
      null_value = g2->as_geometry(result, true /* shallow copy */);
      return g2;
    }

    if (gv1.size() == 1 && gv2.size() == 0) {
      null_value = g1->as_geometry(result, true /* shallow copy */);
      return g1;
    }
  }

  /*
    If both collections have only one basic component, do basic set operation.
    The exception is symdifference with at least one operand being not a
    polygon or multipolygon, in which case this exact function is called to
    perform symdifference for the two basic components.
   */
  if (gv1.size() == 1 && gv2.size() == 1 &&
      (m_spatial_op != op_symdifference ||
       (is_areal(*(gv1.begin())) && is_areal(*(gv2.begin()))))) {
    gres = bg_geo_set_op<Coordsys>(*(gv1.begin()), *(gv2.begin()), result);
    if (null_value) return nullptr;
    if (gres == nullptr && !null_value) {
      gres = empty_result(result, g1->get_srid());
      return gres;
    }

    /*
      If this set operation gives us a gres that's a component/member of either
      bggc1 or bggc2, we have to duplicate the object and its buffer because
      they will be destroyed when bggc1/bggc2 goes out of scope.
     */
    bool do_dup = false;
    for (BG_geometry_collection::Geometry_list::iterator i = gv1.begin();
         i != gv1.end(); ++i)
      if (*i == gres) do_dup = true;
    if (!do_dup)
      for (BG_geometry_collection::Geometry_list::iterator i = gv2.begin();
           i != gv2.end(); ++i)
        if (*i == gres) do_dup = true;

    if (do_dup) {
      String tmpres;
      Geometry *gres2 = nullptr;
      tmpres.append(result->ptr(), result->length());
      const void *data_start = tmpres.ptr() + GEOM_HEADER_SIZE;

      switch (gres->get_geotype()) {
        case Geometry::wkb_point:
          gres2 = new Gis_point;
          break;
        case Geometry::wkb_linestring:
          gres2 = new Gis_line_string;
          break;
        case Geometry::wkb_polygon:
          gres2 = new Gis_polygon;
          break;
        case Geometry::wkb_multipoint:
          gres2 = new Gis_multi_point;
          break;
        case Geometry::wkb_multilinestring:
          gres2 = new Gis_multi_line_string;
          break;
        case Geometry::wkb_multipolygon:
          gres2 = new Gis_multi_polygon;
          break;
        default:
          DBUG_ASSERT(false);
      }

      gres2->set_data_ptr(data_start, tmpres.length() - GEOM_HEADER_SIZE);
      gres2->has_geom_header_space(true);
      gres2->set_bg_adapter(false);
      result->takeover(tmpres);
      gres = gres2;
    }

    return gres;
  }

  switch (m_spatial_op) {
    case op_intersection:
      gres = geocol_intersection<Coordsys>(bggc1, bggc2, result);
      break;
    case op_union:
      gres = geocol_union<Coordsys>(bggc1, bggc2, result);
      break;
    case op_difference:
      gres = geocol_difference<Coordsys>(bggc1, bggc2, result);
      break;
    case op_symdifference:
      gres = geocol_symdifference<Coordsys>(bggc1, bggc2, result);
      break;
    default:
      /* Only above four supported. */
      DBUG_ASSERT(false);
      break;
  }

  if (gres == nullptr && !null_value)
    gres = empty_result(result, g1->get_srid());
  return gres;
}

/**
  Do intersection operation on geometry collections. We do intersection for
  all pairs of components in g1 and g2, put the results in a geometry
  collection. If all subresults can be computed successfully, the geometry
  collection is our result.

  @tparam Coordsys Coordinate system type, specified using those defined in
          boost::geometry::cs.
  @param bggc1 First geometry operand, a geometry collection.
  @param bggc2 Second geometry operand, a geometry collection.
  @param[out] result Holds WKB data of the result, which must be a
                geometry collection.
  @return The intersection result, whose WKB data is stored in 'result'.
 */
template <typename Coordsys>
Geometry *Item_func_spatial_operation::geocol_intersection(
    const BG_geometry_collection &bggc1, const BG_geometry_collection &bggc2,
    String *result) {
  Geometry *gres = nullptr;
  String wkbres;
  Geometry *g0 = nullptr;
  BG_geometry_collection bggc;
  const BG_geometry_collection::Geometry_list &gv1 = bggc1.get_geometries();
  const BG_geometry_collection::Geometry_list &gv2 = bggc2.get_geometries();
  bggc.set_srid(bggc1.get_srid());

  if (gv1.size() == 0 || gv2.size() == 0) {
    return empty_result(result, bggc1.get_srid());
  }

  const typename BG_geometry_collection::Geometry_list *gv = nullptr,
                                                       *gvr = nullptr;

  if (gv1.size() > gv2.size()) {
    gv = &gv2;
    gvr = &gv1;
  } else {
    gv = &gv1;
    gvr = &gv2;
  }

  Rtree_index rtree;
  make_rtree(*gvr, &rtree);

  for (BG_geometry_collection::Geometry_list::const_iterator i = gv->begin();
       i != gv->end(); ++i) {
    BG_box box;
    make_bg_box(*i, &box);

    Rtree_index::const_query_iterator j = rtree.qbegin(bgi::intersects(box));
    if (j == rtree.qend()) continue;

    for (; j != rtree.qend(); ++j) {
      Geometry *geom = (*gvr)[j->second];
      // Free before using it, wkbres may have WKB data from last execution.
      wkbres.mem_free();
      wkbres.length(0);
      g0 = bg_geo_set_op<Coordsys>(*i, geom, &wkbres);

      if (null_value) {
        if (g0 != nullptr && g0 != *i && g0 != geom) delete g0;
        return nullptr;
      }

      if (g0 && !is_empty_geocollection(wkbres)) bggc.fill(g0);
      if (g0 != nullptr && g0 != *i && g0 != geom) {
        delete g0;
        g0 = nullptr;
      }
    }
  }

  /*
    Note: result unify and merge

    The result may have geometry elements that overlap, caused by overlap
    geos in either or both gc1 and/or gc2. Also, there may be geometries
    that can be merged into a larger one of the same type in the result.
    We will need to figure out how to make such enhancements.
   */
  bggc.merge_components<Coordsys>(&null_value);
  if (null_value) return nullptr;
  gres = bggc.as_geometry_collection(result);

  return gres;
}

/**
  Do union operation on geometry collections. We do union for
  all pairs of components in g1 and g2, whenever a union can be done, we do
  so and put the results in a geometry collection GC and remove the two
  components from g1 and g2 respectively. Finally no components in g1 and g2
  overlap and GC is our result.

  @tparam Coordsys Coordinate system type, specified using those defined in
          boost::geometry::cs.
  @param bggc1 First geometry operand, a geometry collection.
  @param bggc2 Second geometry operand, a geometry collection.
  @param[out] result Holds WKB data of the result, which must be a
                geometry collection.
  @return The union result, whose WKB data is stored in 'result'.
 */
template <typename Coordsys>
Geometry *Item_func_spatial_operation::geocol_union(
    const BG_geometry_collection &bggc1, const BG_geometry_collection &bggc2,
    String *result) {
  Geometry *gres = nullptr;
  BG_geometry_collection bggc;
  BG_geometry_collection::Geometry_list &gv = bggc.get_geometries();
  gv.insert(gv.end(), bggc1.get_geometries().begin(),
            bggc1.get_geometries().end());
  gv.insert(gv.end(), bggc2.get_geometries().begin(),
            bggc2.get_geometries().end());
  bggc.set_srid(bggc1.get_srid());

  // It's likely that there are overlapping components in bggc because it
  // has components from both bggc1 and bggc2.
  bggc.merge_components<Coordsys>(&null_value);
  if (!null_value) gres = bggc.as_geometry_collection(result);

  return gres;
}

/**
  Do difference operation on geometry collections. For each component CX in g1,
  we do CX:= CX difference CY for all components CY in g2. When at last CX isn't
  empty, it's put into result geometry collection GC.
  If all subresults can be computed successfully, the geometry
  collection GC is our result.

  @tparam Coordsys Coordinate system type, specified using those defined in
          boost::geometry::cs.
  @param bggc1 First geometry operand, a geometry collection.
  @param bggc2 Second geometry operand, a geometry collection.
  @param[out] result Holds WKB data of the result, which must be a
                geometry collection.
  @return The difference result, whose WKB data is stored in 'result'.
 */
template <typename Coordsys>
Geometry *Item_func_spatial_operation::geocol_difference(
    const BG_geometry_collection &bggc1, const BG_geometry_collection &bggc2,
    String *result) {
  Geometry *gres = nullptr;
  String *wkbres = nullptr;
  BG_geometry_collection bggc;
  const BG_geometry_collection::Geometry_list *gv1 = &(bggc1.get_geometries());
  const BG_geometry_collection::Geometry_list *gv2 = &(bggc2.get_geometries());

  bggc.set_srid(bggc1.get_srid());

  // Difference isn't symetric so we have to always build rtree tndex on gv2.
  Rtree_index rtree;
  make_rtree(*gv2, &rtree);

  for (BG_geometry_collection::Geometry_list::const_iterator i = gv1->begin();
       i != gv1->end(); ++i) {
    bool g11_isempty = false;
    std::unique_ptr<Geometry> guard11;
    Geometry *g11 = nullptr;
    g11 = *i;
    Inplace_vector<String> wkbstrs(PSI_INSTRUMENT_ME);

    BG_box box;
    make_bg_box(*i, &box);

    /*
      Above theory makes sure all results are in rtree_result, the logic
      here is sufficient when rtree_result is empty.
    */
    for (Rtree_index::const_query_iterator j =
             rtree.qbegin(bgi::intersects(box));
         j != rtree.qend(); ++j) {
      Geometry *geom = (*gv2)[j->second];

      wkbres = wkbstrs.append_object();
      if (wkbres == nullptr) {
        null_value = true;
        return nullptr;
      }
      Geometry *g0 = bg_geo_set_op<Coordsys>(g11, geom, wkbres);
      std::unique_ptr<Geometry> guard0(g0);

      if (null_value) {
        if (!(g0 != nullptr && g0 != *i && g0 != geom)) guard0.release();
        if (!(g11 != nullptr && g11 != g0 && g11 != *i && g11 != geom))
          guard11.release();
        return nullptr;
      }

      if (g0 != nullptr && !is_empty_geocollection(*wkbres)) {
        if (g11 != nullptr && g11 != *i && g11 != geom && g11 != g0)
          delete guard11.release();
        else
          guard11.release();
        guard0.release();
        g11 = g0;
        if (g0 != nullptr && g0 != *i && g0 != geom) guard11.reset(g11);
      } else {
        g11_isempty = true;
        if (!(g0 != nullptr && g0 != *i && g0 != geom && g0 != g11))
          guard0.release();
        break;
      }
    }

    if (!g11_isempty) bggc.fill(g11);
    if (!(g11 != nullptr && g11 != *i))
      guard11.release();
    else
      guard11.reset(nullptr);
  }

  bggc.merge_components<Coordsys>(&null_value);
  if (null_value) return nullptr;
  gres = bggc.as_geometry_collection(result);

  return gres;
}

/**
  Symmetric difference of geometry collections.

  Symdifference(g1, g2) is implemented as
  union(difference(g1, g2), difference(g2, g1)).

  @tparam Coordsys Coordinate system type, specified using those defined in
          boost::geometry::cs.
  @param bggc1 First geometry operand, a geometry collection.
  @param bggc2 Second geometry operand, a geometry collection.
  @param[out] result Holds WKB data of the result, which must be a
                geometry collection.
  @return The symdifference result, whose WKB data is stored in 'result'.
 */
template <typename Coordsys>
Geometry *Item_func_spatial_operation::geocol_symdifference(
    const BG_geometry_collection &bggc1, const BG_geometry_collection &bggc2,
    String *result) {
  Geometry *res = nullptr;
  std::unique_ptr<Geometry> diff12;
  std::unique_ptr<Geometry> diff21;
  String diff12_wkb;
  String diff21_wkb;

  Var_resetter<op_type> var_reset(&m_spatial_op, op_symdifference);

  m_spatial_op = op_difference;
  diff12.reset(geocol_difference<Coordsys>(bggc1, bggc2, &diff12_wkb));
  if (null_value) return nullptr;
  DBUG_ASSERT(diff12.get() != nullptr);

  diff21.reset(geocol_difference<Coordsys>(bggc2, bggc1, &diff21_wkb));
  if (null_value) return nullptr;
  DBUG_ASSERT(diff21.get() != nullptr);

  m_spatial_op = op_union;
  res = geometry_collection_set_operation<Coordsys>(diff12.get(), diff21.get(),
                                                    result);
  if (diff12.get() == res) {
    result->takeover(diff12_wkb);
    diff12.release();
  } else if (diff21.get() == res) {
    result->takeover(diff21_wkb);
    diff21.release();
  }

  if (null_value) {
    if (res != nullptr) delete res;
    return nullptr;
  }

  return res;
}

bool Item_func_spatial_operation::assign_result(Geometry *geo, String *result) {
  DBUG_ASSERT(geo->has_geom_header_space());
  char *p = geo->get_cptr() - GEOM_HEADER_SIZE;
  write_geometry_header(p, geo->get_srid(), geo->get_geotype());
  result->set(p, GEOM_HEADER_SIZE + geo->get_nbytes(), &my_charset_bin);
  m_bg_resbuf_mgr.add_buffer(p);
  geo->set_ownmem(false);

  return false;
}
