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

#include <stddef.h>
#include <boost/concept/usage.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/derror.h"  // ER_THD
#include "sql/gis/geometries.h"
#include "sql/gis/relops.h"
#include "sql/gis/srid.h"
#include "sql/gis/wkb.h"
#include "sql/item.h"
#include "sql/item_cmpfunc.h"
#include "sql/item_func.h"
#include "sql/item_geofunc.h"
#include "sql/item_geofunc_internal.h"
#include "sql/item_geofunc_relchecks_bgwrap.h"
#include "sql/spatial.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_error.h"
#include "sql/sql_exception_handler.h"
#include "sql/srs_fetcher.h"
#include "sql_string.h"

namespace boost {
namespace geometry {
namespace cs {
struct cartesian;
}  // namespace cs
}  // namespace geometry
}  // namespace boost

/*
  Functions for spatial relations
*/

const char *Item_func_spatial_mbr_rel::func_name() const {
  switch (spatial_rel) {
    case SP_CONTAINS_FUNC:
      return "mbrcontains";
    case SP_WITHIN_FUNC:
      return "mbrwithin";
    case SP_EQUALS_FUNC:
      return "mbrequals";
    case SP_DISJOINT_FUNC:
      return "mbrdisjoint";
    case SP_INTERSECTS_FUNC:
      return "mbrintersects";
    case SP_TOUCHES_FUNC:
      return "mbrtouches";
    case SP_CROSSES_FUNC:
      return "mbrcrosses";
    case SP_OVERLAPS_FUNC:
      return "mbroverlaps";
    case SP_COVERS_FUNC:
      return "mbrcovers";
    case SP_COVEREDBY_FUNC:
      return "mbrcoveredby";
    default:
      DBUG_ASSERT(0);  // Should never happened
      return "mbrsp_unknown";
  }
}

longlong Item_func_spatial_mbr_rel::val_int() {
  DBUG_ASSERT(fixed == 1);
  String *res1 = args[0]->val_str(&cmp.value1);
  String *res2 = args[1]->val_str(&cmp.value2);
  Geometry_buffer buffer1, buffer2;
  Geometry *g1, *g2;
  MBR mbr1, mbr2;

  if ((null_value =
           (!res1 || args[0]->null_value || !res2 || args[1]->null_value)))
    return 0;
  if (!(g1 = Geometry::construct(&buffer1, res1)) ||
      !(g2 = Geometry::construct(&buffer2, res2))) {
    my_error(ER_GIS_INVALID_DATA, MYF(0), func_name());
    return error_int();
  }
  if ((null_value = (g1->get_mbr(&mbr1) || g2->get_mbr(&mbr2)))) return 0;

  // The two geometry operands must be in the same coordinate system.
  if (g1->get_srid() != g2->get_srid()) {
    my_error(ER_GIS_DIFFERENT_SRIDS, MYF(0), func_name(), g1->get_srid(),
             g2->get_srid());
    null_value = true;
    return 0;
  }

  if (g1->get_srid() != 0) {
    bool srs_exists = false;
    if (Srs_fetcher::srs_exists(current_thd, g1->get_srid(), &srs_exists))
      return error_int();  // Error has already been flagged.

    if (!srs_exists) {
      push_warning_printf(
          current_thd, Sql_condition::SL_WARNING, ER_WARN_SRS_NOT_FOUND,
          ER_THD(current_thd, ER_WARN_SRS_NOT_FOUND), g1->get_srid());
    }
  }

  int ret = 0;

  switch (spatial_rel) {
    case SP_CONTAINS_FUNC:
      ret = mbr1.contains(&mbr2);
      break;
    case SP_WITHIN_FUNC:
      ret = mbr1.within(&mbr2);
      break;
    case SP_EQUALS_FUNC:
      ret = mbr1.equals(&mbr2);
      break;
    case SP_DISJOINT_FUNC:
      ret = mbr1.disjoint(&mbr2);
      break;
    case SP_INTERSECTS_FUNC:
      ret = mbr1.intersects(&mbr2);
      break;
    case SP_TOUCHES_FUNC:
      ret = mbr1.touches(&mbr2);
      break;
    case SP_OVERLAPS_FUNC:
      ret = mbr1.overlaps(&mbr2);
      break;
    case SP_CROSSES_FUNC:
      DBUG_ASSERT(false);
      ret = 0;
      null_value = true;
      break;
    case SP_COVERS_FUNC:
      ret = mbr1.covers(&mbr2);
      break;
    case SP_COVEREDBY_FUNC:
      ret = mbr1.covered_by(&mbr2);
      break;
    default:
      break;
  }

  if (ret == -1) {
    my_error(ER_GIS_INVALID_DATA, MYF(0), func_name());
    return error_int();
  }

  return ret;
}

longlong Item_func_spatial_rel::val_int() {
  DBUG_TRACE;
  DBUG_ASSERT(fixed == 1);
  String tmp_value1;
  String tmp_value2;
  String *res1 = nullptr;
  String *res2 = nullptr;
  Geometry_buffer buffer1, buffer2;
  Geometry *g1 = nullptr, *g2 = nullptr;
  int tres = 0;
  bool had_except = false;
  bool had_error = false;
  String wkt1, wkt2;

  res1 = args[0]->val_str(&tmp_value1);
  res2 = args[1]->val_str(&tmp_value2);
  if ((null_value =
           (!res1 || args[0]->null_value || !res2 || args[1]->null_value)))
    goto exit;
  if (!(g1 = Geometry::construct(&buffer1, res1)) ||
      !(g2 = Geometry::construct(&buffer2, res2))) {
    my_error(ER_GIS_INVALID_DATA, MYF(0), func_name());
    tres = error_int();
    goto exit;
  }

  // The two geometry operands must be in the same coordinate system.
  if (g1->get_srid() != g2->get_srid()) {
    my_error(ER_GIS_DIFFERENT_SRIDS, MYF(0), func_name(), g1->get_srid(),
             g2->get_srid());
    tres = error_int();
    goto exit;
  }

  if (g1->get_srid() != 0) {
    bool srs_exists = false;
    if (Srs_fetcher::srs_exists(current_thd, g1->get_srid(), &srs_exists))
      return error_int();  // Error has already been flagged.

    if (!srs_exists) {
      push_warning_printf(
          current_thd, Sql_condition::SL_WARNING, ER_WARN_SRS_NOT_FOUND,
          ER_THD(current_thd, ER_WARN_SRS_NOT_FOUND), g1->get_srid());
    }
  }

  /*
    Catch all exceptions to make sure no exception can be thrown out of
    current function. Put all and any code that calls Boost.Geometry functions,
    STL functions into this try block. Code out of the try block should never
    throw any exception.
  */
  try {
    if (g1->get_type() != Geometry::wkb_geometrycollection &&
        g2->get_type() != Geometry::wkb_geometrycollection) {
      // Must use double, otherwise may lose valid result, not only precision.
      tres = bg_geo_relation_check(g1, g2, functype(), &had_error);
    } else
      tres = geocol_relation_check<bgcs::cartesian>(g1, g2);
  } catch (...) {
    had_except = true;
    handle_gis_exception(func_name());
  }

  if (had_except || had_error || null_value) return error_int();

exit:
  return tres;
}

/**
  Do geometry collection relation check. Boost geometry doesn't support
  geometry collections directly, we have to treat them as a collection of basic
  geometries and use BG features to compute.
  @tparam Coordsys Coordinate system type, specified using those defined in
          boost::geometry::cs.
  @param g1 the 1st geometry collection parameter.
  @param g2 the 2nd geometry collection parameter.
  @return whether g1 and g2 satisfy the specified relation, 0 for negative,
                none 0 for positive.
 */
template <typename Coordsys>
int Item_func_spatial_rel::geocol_relation_check(Geometry *g1, Geometry *g2) {
  String gcbuf;
  Geometry *tmpg = nullptr;
  int tres = 0;
  const typename BG_geometry_collection::Geometry_list *gv1 = nullptr,
                                                       *gv2 = nullptr;
  BG_geometry_collection bggc1, bggc2;
  bool empty1 = is_empty_geocollection(g1);
  bool empty2 = is_empty_geocollection(g2);
  Var_resetter<enum Functype> resetter;
  enum Functype spatial_rel = functype();

  /*
    An empty geometry collection is an empty point set, according to OGC
    specifications and set theory we make below conclusion.
   */
  if (empty1 || empty2) {
    if (spatial_rel == SP_DISJOINT_FUNC)
      tres = 1;
    else if (empty1 && empty2 && spatial_rel == SP_EQUALS_FUNC)
      tres = 1;
    return tres;
  }

  if (spatial_rel == SP_CONTAINS_FUNC) {
    tmpg = g2;
    g2 = g1;
    g1 = tmpg;
    spatial_rel = SP_WITHIN_FUNC;
    resetter.set(&spatial_rel, SP_CONTAINS_FUNC);
  }

  bggc1.fill(g1);
  bggc2.fill(g2);

  /*
    When checking GC1 within GC2, we want GC1 to be disintegrated pieces
    rather than merging its components to larger pieces, because a
    multi-geometry of GC1 may consist of multiple components which are within
    different components of GC2, but if merged, it would not be within any
    component of GC2.
   */
  if (spatial_rel != SP_WITHIN_FUNC)
    bggc1.merge_components<Coordsys>(&null_value);
  if (null_value) return tres;
  bggc2.merge_components<Coordsys>(&null_value);
  if (null_value) return tres;

  gv1 = &(bggc1.get_geometries());
  gv2 = &(bggc2.get_geometries());

  if (gv1->size() == 0 || gv2->size() == 0) {
    null_value = true;
    return tres;
  } else if (gv1->size() == 1 && gv2->size() == 1) {
    tres = bg_geo_relation_check(*(gv1->begin()), *(gv2->begin()), spatial_rel,
                                 &null_value);
    return tres;
  }

  if (spatial_rel == SP_OVERLAPS_FUNC || spatial_rel == SP_CROSSES_FUNC ||
      spatial_rel == SP_TOUCHES_FUNC) {
    /*
      OGC says this is not applicable, and we always return false for
      inapplicable situations.
    */
    return 0;
  }

  if (spatial_rel == SP_DISJOINT_FUNC || spatial_rel == SP_INTERSECTS_FUNC)
    tres = geocol_relcheck_intersect_disjoint(gv1, gv2);
  else if (spatial_rel == SP_WITHIN_FUNC)
    tres = geocol_relcheck_within<Coordsys>(gv1, gv2, spatial_rel);
  else if (spatial_rel == SP_EQUALS_FUNC)
    tres = geocol_equals_check<Coordsys>(gv1, gv2);
  else
    DBUG_ASSERT(false);

  /* If doing contains check, need to switch back the two operands. */
  if (tmpg) {
    DBUG_ASSERT(spatial_rel == SP_WITHIN_FUNC);
    tmpg = g2;
    g2 = g1;
    g1 = tmpg;
  }

  return tres;
}

/**
  Geometry collection relation checks for disjoint and intersects operations.

  @param gv1 the 1st geometry collection parameter.
  @param gv2 the 2nd geometry collection parameter.
  @return whether @p gv1 and @p gv2 satisfy the specified relation, 0 for
  negative, none 0 for positive.
 */
int Item_func_spatial_rel::geocol_relcheck_intersect_disjoint(
    const BG_geometry_collection::Geometry_list *gv1,
    const BG_geometry_collection::Geometry_list *gv2) {
  int tres = 0;

  DBUG_ASSERT(functype() == SP_DISJOINT_FUNC ||
              functype() == SP_INTERSECTS_FUNC);
  const BG_geometry_collection::Geometry_list *gv = nullptr, *gvr = nullptr;

  if (gv1->size() > gv2->size()) {
    gv = gv2;
    gvr = gv1;
  } else {
    gv = gv1;
    gvr = gv2;
  }

  Rtree_index rtree;
  make_rtree(*gvr, &rtree);

  for (BG_geometry_collection::Geometry_list::const_iterator i = gv->begin();
       i != gv->end(); ++i) {
    tres = 0;

    BG_box box;
    make_bg_box(*i, &box);
    for (Rtree_index::const_query_iterator j =
             rtree.qbegin(bgi::intersects(box));
         j != rtree.qend(); ++j) {
      bool had_except = false;
      bool had_error = false;

      try {
        tres = bg_geo_relation_check(*i, (*gvr)[j->second], functype(),
                                     &had_error);
      } catch (...) {
        had_except = true;
        handle_gis_exception(func_name());
      }

      if (had_except || had_error) return error_int();

      if (null_value) return tres;

      /*
        If a pair of geometry intersect or don't disjoint, the two
        geometry collections intersect or don't disjoint, in both cases the
        check is completed.
       */
      if ((functype() == SP_INTERSECTS_FUNC && tres) ||
          (functype() == SP_DISJOINT_FUNC && !tres))
        return tres;
    }
  }

  /*
    When we arrive here, the disjoint check must have succeeded and
    intersects check must have failed, otherwise control would
    have gone out of this function.

    The reason we can derive the relation check result is that if
    any two geometries from the two collections intersect, the two
    geometry collections intersect; and disjoint is true
    only when any(and every) combination of geometries from
    the two collections are disjoint.

    tres can be either true or false for DISJOINT check because the inner
    loop may never executed and tres woule be false.
   */
  DBUG_ASSERT(functype() == SP_DISJOINT_FUNC ||
              (!tres && functype() == SP_INTERSECTS_FUNC));
  return tres;
}

/**
  Multipoint need special handling because for a multipoint MP to be
  within geometry G, only one point in MP has to be 'within' G,
  the rest only need to intersect G.

  @param pmpts the multipoint to check.
  @param gv2 the geometry collection's component list.
  @param prtree the rtree index built on gv2. We can't expose the
  Rtree_index type in item_geofunc.h so have to use the generic void* type.
  This function is called where an rtree index on gv2 is already built so
  we want to pass it in to avoid unnecessarily build the same one again.
 */
template <typename Coordsys>
int Item_func_spatial_rel::multipoint_within_geometry_collection(
    Gis_multi_point *pmpts,
    const typename BG_geometry_collection::Geometry_list *gv2,
    const void *prtree) {
  int has_inner = 0;
  int tres = 0;
  bool had_error = false;

  const Rtree_index &rtree = *static_cast<const Rtree_index *>(prtree);

  typename BG_models<Coordsys>::Multipoint mpts(
      pmpts->get_data_ptr(), pmpts->get_data_size(), pmpts->get_flags(),
      pmpts->get_srid());

  for (typename BG_models<Coordsys>::Multipoint::iterator k = mpts.begin();
       k != mpts.end(); ++k) {
    bool already_in = false;
    BG_box box;
    make_bg_box(&(*k), &box);

    /*
      Search for geometries in gv2 that may intersect *k point using the
      rtree index.
      All geometries that possibly intersect *k point are given by the
      rtree iteration below.
    */
    for (Rtree_index::const_query_iterator j =
             rtree.qbegin(bgi::intersects(box));
         j != rtree.qend(); ++j) {
      /*
        If we don't have a point in mpts that's within a component of gv2 yet,
        check whether *k is within *j.
        If *k is within *j, it's already in the geometry collection gv2,
        so no need for more checks for the point *k, get out of the iteration.
      */
      if (!has_inner) {
        tres = bg_geo_relation_check(&(*k), (*gv2)[j->second], SP_WITHIN_FUNC,
                                     &had_error);
        if (had_error || null_value) return error_int();
        if ((has_inner = tres)) {
          already_in = true;
          break;
        }
      }

      /*
        If we already have a point within gv2, OR if *k is checked above to
        be not within *j, check whether *k intersects *j.
        *k has to intersect one of the components in this loop, otherwise *k
        is out of gv2.
       */
      tres = bg_geo_relation_check(&(*k), (*gv2)[j->second], SP_INTERSECTS_FUNC,
                                   &had_error);
      if (had_error || null_value) return error_int();

      if (tres) {
        already_in = true;
        /*
          It's likely that *k is within another geometry, so only stop the
          iteration if we already have a point that's within gv2,
          in order not to miss the potential geometry containing *k.
        */
        if (has_inner) break;
      }
    }

    /*
      The *k point isn't within or intersects any geometry compoennt of gv2,
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

/**
  Geometry collection relation checks for within and equals(half) checks.

  @tparam Coordsys Coordinate system type, specified using those defined in
          boost::geometry::cs.
  @param gv1 the 1st geometry collection parameter.
  @param gv2 the 2nd geometry collection parameter.
  @param spatial_rel The spatial relational operator to test (within or equals).
  @return whether @p gv1 and @p gv2 satisfy the specified relation, 0 for
  negative, none 0 for positive.
 */
template <typename Coordsys>
int Item_func_spatial_rel::geocol_relcheck_within(
    const typename BG_geometry_collection::Geometry_list *gv1,
    const typename BG_geometry_collection::Geometry_list *gv2,
    enum Functype spatial_rel MY_ATTRIBUTE((unused))) {
  int tres = 0;

  /*
    When this function is called by geocol_equals_check,this is true:
    spatial_rel == SP_EQUALS_FUNC
    But even in this case, in this function we still want to check each
    component of gv1 is within gv2, so in this function we always assume
    with check and and use SP_WITHIN_FUNC.
  */
  DBUG_ASSERT(spatial_rel == SP_WITHIN_FUNC || spatial_rel == SP_EQUALS_FUNC);

  // Within isn't symetric so we have to always build rtree tndex on gv2.
  Rtree_index rtree;
  make_rtree(*gv2, &rtree);

  BG_geometry_collection bggc;
  bool no_fill = true;

  /*
    We have to break any multi-geometry into its components before the within
    check, because the components of some multi-geometry MG in gv1 may be in
    different geometries of gv2, and in all the MG is still in gv2.
    Without the disintegration, MG would be seen as not within gv2.

    Multipoint need special handling because for a multipoint MP to be within
    geometry G, only one point in MP has to be 'within' G, the rest only need
    to intersect G.
  */
  for (size_t i = 0; i < gv1->size(); i++) {
    Geometry::wkbType gtype = (*gv1)[i]->get_type();
    if (gtype == Geometry::wkb_multipolygon ||
        gtype == Geometry::wkb_multilinestring) {
      if (no_fill) {
        for (size_t j = 0; j < i; j++) bggc.fill((*gv1)[j]);
        no_fill = false;
      }

      bggc.fill((*gv1)[i], true /* break multi-geometry. */);
    } else if (!no_fill)
      bggc.fill((*gv1)[i]);
  }

  if (!no_fill) gv1 = &(bggc.get_geometries());

  for (BG_geometry_collection::Geometry_list::const_iterator i = gv1->begin();
       i != gv1->end(); ++i) {
    bool innerOK = false;
    tres = 0;

    if ((*i)->get_type() == Geometry::wkb_multipoint) {
      Gis_multi_point *mpts = static_cast<Gis_multi_point *>(*i);
      tres = multipoint_within_geometry_collection<Coordsys>(mpts, gv2, &rtree);
      if (null_value) return error_int();
      if (tres)
        continue;
      else
        return tres;
    }

    /*
      Why it works to scan rtree index for within check? Because of the below
      conclusions.

      1. g1 within g2 => MBR(g1) within MBR(g2)
      Proof:
      Suppose MBR(g1) not within MBR(g2), then there exists a point P in g1
      such that either P.x not in MBR(g2)'s horizontal range, or P.y not in
      MBR(g2)'s vertical range. Since both ranges are limits of g2 too,
      that means P isn't in g2. Similarly we can have below conclusion for
      contains.

      2. g1 contains g2 => MBR(g1) contains MBR(g2)

      That is to say, MBR(g1) within/contains MBR(g2) is the necessary but not
      sufficient condition for g1 within/contains g2. All possible final result
      are in the ones returned by the rtree query.
     */

    BG_box box;
    make_bg_box(*i, &box);

    /*
      Above theory makes sure all results are in rtree search result, the logic
      here is sufficient when the result is empty.
    */
    for (Rtree_index::const_query_iterator j = rtree.qbegin(bgi::covers(box));
         j != rtree.qend(); ++j) {
      bool had_except = false;
      bool had_error = false;

      try {
        tres = bg_geo_relation_check(*i, (*gv2)[j->second], SP_WITHIN_FUNC,
                                     &had_error);
      } catch (...) {
        had_except = true;
        handle_gis_exception(func_name());
      }

      if (had_except || had_error || null_value) return error_int();

      /*
        We've found a geometry j in gv2 so that current geometry element i
        in gv1 is within j, or i is equal to j. This means i in gv1
        passes the test, proceed to next geometry in gv1.
       */
      if (tres) {
        innerOK = true;
        break;
      }
    }

    /*
      For within and equals check, if we can't find a geometry j in gv2
      so that current geometry element i in gv1 is with j or i is equal to j,
      gv1 is not within or equal to gv2.
     */
    if (!innerOK) {
      DBUG_ASSERT(tres == 0);
      return tres;
    }
  }

  /*
    When we arrive here, within or equals checks must have
    succeeded, otherwise control would go out of this function.
    The reason we can derive the relation check result is that
    within and equals are true only when any(and every) combination of
    geometries from the two collections are true for the relation check.
   */
  DBUG_ASSERT(tres);

  return tres;
}

/**
  Geometry collection equality check.
  @tparam Coordsys Coordinate system type, specified using those defined in
          boost::geometry::cs.
  @param gv1 the 1st geometry collection parameter.
  @param gv2 the 2nd geometry collection parameter.
  @return whether @p gv1 and @p gv2 satisfy the specified relation, 0 for
  negative, none 0 for positive.
 */
template <typename Coordsys>
int Item_func_spatial_rel::geocol_equals_check(
    const typename BG_geometry_collection::Geometry_list *gv1,
    const typename BG_geometry_collection::Geometry_list *gv2) {
  int tres = 0, num_try = 0;
  DBUG_ASSERT(functype() == SP_EQUALS_FUNC);

  do {
    tres = geocol_relcheck_within<Coordsys>(gv1, gv2, functype());
    if (!tres || null_value) return tres;
    /*
      Two sets A and B are equal means A is a subset of B and B is a
      subset of A. Thus we need to check twice, each successful check
      means half truth. Switch gv1 and gv2 for 2nd check.
     */
    std::swap(gv1, gv2);
    num_try++;
  } while (num_try < 2);

  return tres;
}

/**
  Do within relation check of two geometries.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
*/
template <typename Geom_types>
int Item_func_spatial_rel::within_check(Geometry *g1, Geometry *g2,
                                        bool *pnull_value) {
  Geometry::wkbType gt1;
  int result = 0;

  gt1 = g1->get_type();

  if (gt1 == Geometry::wkb_point)
    result = BG_wrap<Geom_types>::point_within_geometry(g1, g2, pnull_value);
  else if (gt1 == Geometry::wkb_multipoint)
    result =
        BG_wrap<Geom_types>::multipoint_within_geometry(g1, g2, pnull_value);
  else if (gt1 == Geometry::wkb_linestring)
    result =
        BG_wrap<Geom_types>::linestring_within_geometry(g1, g2, pnull_value);
  else if (gt1 == Geometry::wkb_multilinestring)
    result = BG_wrap<Geom_types>::multilinestring_within_geometry(g1, g2,
                                                                  pnull_value);
  else if (gt1 == Geometry::wkb_polygon)
    result = BG_wrap<Geom_types>::polygon_within_geometry(g1, g2, pnull_value);
  else if (gt1 == Geometry::wkb_multipolygon)
    result =
        BG_wrap<Geom_types>::multipolygon_within_geometry(g1, g2, pnull_value);
  else
    DBUG_ASSERT(false);
  return result;
}

/**
  Do equals relation check of two geometries.
  Dispatch to specific BG functions according to operation type, and 1st or
  both operand types.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
*/
template <typename Geom_types>
int Item_func_spatial_rel::equals_check(Geometry *g1, Geometry *g2,
                                        bool *pnull_value) {
  typedef typename Geom_types::Point Point;
  typedef typename Geom_types::Linestring Linestring;
  typedef typename Geom_types::Multilinestring Multilinestring;
  typedef typename Geom_types::Polygon Polygon;
  typedef typename Geom_types::Multipoint Multipoint;
  typedef typename Geom_types::Multipolygon Multipolygon;
  typedef std::set<Point, bgpt_lt> Point_set;

  int result = 0;
  Geometry::wkbType gt1 = g1->get_type();
  Geometry::wkbType gt2 = g2->get_type();

  /*
    Only geometries of the same base type can be equal, any other
    combinations always result as false. This is different from all other types
    of geometry relation checks.
   */
  if (gt1 == Geometry::wkb_point) {
    if (gt2 == Geometry::wkb_point)
      BGCALL(result, equals, Point, g1, Point, g2, pnull_value);
    else if (gt2 == Geometry::wkb_multipoint) {
      Point pt(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
               g1->get_srid());
      Multipoint mpts(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                      g2->get_srid());

      Point_set ptset(mpts.begin(), mpts.end());

      result =
          (ptset.size() == 1 && boost::geometry::equals(pt, *ptset.begin()));
    } else
      result = 0;
  } else if (gt1 == Geometry::wkb_multipoint)
    result =
        BG_wrap<Geom_types>::multipoint_equals_geometry(g1, g2, pnull_value);
  else if (gt1 == Geometry::wkb_linestring && gt2 == Geometry::wkb_linestring)
    BGCALL(result, equals, Linestring, g1, Linestring, g2, pnull_value);
  else if (gt1 == Geometry::wkb_linestring &&
           gt2 == Geometry::wkb_multilinestring)
    BGCALL(result, equals, Linestring, g1, Multilinestring, g2, pnull_value);
  else if (gt2 == Geometry::wkb_linestring &&
           gt1 == Geometry::wkb_multilinestring)
    BGCALL(result, equals, Multilinestring, g1, Linestring, g2, pnull_value);
  else if (gt2 == Geometry::wkb_multilinestring &&
           gt1 == Geometry::wkb_multilinestring)
    BGCALL(result, equals, Multilinestring, g1, Multilinestring, g2,
           pnull_value);
  else if (gt1 == Geometry::wkb_polygon && gt2 == Geometry::wkb_polygon)
    BGCALL(result, equals, Polygon, g1, Polygon, g2, pnull_value);
  else if (gt1 == Geometry::wkb_polygon && gt2 == Geometry::wkb_multipolygon)
    BGCALL(result, equals, Polygon, g1, Multipolygon, g2, pnull_value);
  else if (gt1 == Geometry::wkb_multipolygon && gt2 == Geometry::wkb_polygon)
    BGCALL(result, equals, Multipolygon, g1, Polygon, g2, pnull_value);
  else if (gt1 == Geometry::wkb_multipolygon &&
           gt2 == Geometry::wkb_multipolygon)
    BGCALL(result, equals, Multipolygon, g1, Multipolygon, g2, pnull_value);
  else
    /* This branch covers all the unequal dimension combinations. */
    result = 0;
  return result;
}

/**
  Do disjoint relation check of two geometries.
  Dispatch to specific BG functions according to operation type, and 1st or
  both operand types.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
*/
template <typename Geom_types>
int Item_func_spatial_rel::disjoint_check(Geometry *g1, Geometry *g2,
                                          bool *pnull_value) {
  Geometry::wkbType gt1;
  int result = 0;

  gt1 = g1->get_type();

  switch (gt1) {
    case Geometry::wkb_point:
      result =
          BG_wrap<Geom_types>::point_disjoint_geometry(g1, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint:
      result = BG_wrap<Geom_types>::multipoint_disjoint_geometry(g1, g2,
                                                                 pnull_value);
      break;
    case Geometry::wkb_linestring:
      result = BG_wrap<Geom_types>::linestring_disjoint_geometry(g1, g2,
                                                                 pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      result = BG_wrap<Geom_types>::multilinestring_disjoint_geometry(
          g1, g2, pnull_value);
      break;
    case Geometry::wkb_polygon:
      result =
          BG_wrap<Geom_types>::polygon_disjoint_geometry(g1, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      result = BG_wrap<Geom_types>::multipolygon_disjoint_geometry(g1, g2,
                                                                   pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }

  /*
    Note: need disjoint(point, Linestring) and disjoint(linestring, Polygon)
   */
  return result;
}

/**
  Do interesects relation check of two geometries.
  Dispatch to specific BG functions according to operation type, and 1st or
  both operand types.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
*/
template <typename Geom_types>
int Item_func_spatial_rel::intersects_check(Geometry *g1, Geometry *g2,
                                            bool *pnull_value) {
  Geometry::wkbType gt1;
  int result = 0;

  gt1 = g1->get_type();
  /*
    According to OGC SFA, intersects is identical to !disjoint, but
    boost geometry has functions to compute intersects, so we still call
    them.
   */
  switch (gt1) {
    case Geometry::wkb_point:
      result =
          BG_wrap<Geom_types>::point_intersects_geometry(g1, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint:
      result = BG_wrap<Geom_types>::multipoint_intersects_geometry(g1, g2,
                                                                   pnull_value);
      break;
    case Geometry::wkb_linestring:
      result = BG_wrap<Geom_types>::linestring_intersects_geometry(g1, g2,
                                                                   pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      result = BG_wrap<Geom_types>::multilinestring_intersects_geometry(
          g1, g2, pnull_value);
      break;
    case Geometry::wkb_polygon:
      result =
          BG_wrap<Geom_types>::polygon_intersects_geometry(g1, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      result = BG_wrap<Geom_types>::multipolygon_intersects_geometry(
          g1, g2, pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }
  /*
    Note: need intersects(pnt, lstr), (lstr, plgn)
   */
  return result;
}

/**
  Do overlaps relation check of two geometries.
  Dispatch to specific BG functions according to operation type, and 1st or
  both operand types.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
*/
template <typename Geom_types>
int Item_func_spatial_rel::overlaps_check(Geometry *g1, Geometry *g2,
                                          bool *pnull_value) {
  typedef typename Geom_types::Linestring Linestring;
  typedef typename Geom_types::Multilinestring Multilinestring;
  typedef typename Geom_types::Polygon Polygon;
  typedef typename Geom_types::Multipolygon Multipolygon;

  int result = 0;
  Geometry::wkbType gt1 = g1->get_type();
  Geometry::wkbType gt2 = g2->get_type();

  if (g1->feature_dimension() != g2->feature_dimension()) {
    /*
      OGC says this is not applicable, and we always return false for
      inapplicable situations.
    */
    return 0;
  }

  if (gt1 == Geometry::wkb_point || gt2 == Geometry::wkb_point) return 0;

  if (gt1 == Geometry::wkb_multipoint && gt2 == Geometry::wkb_multipoint) {
    result = BG_wrap<Geom_types>::multipoint_overlaps_multipoint(g1, g2);
    return result;
  }

  switch (gt1) {
    case Geometry::wkb_linestring: {
      switch (gt2) {
        case Geometry::wkb_linestring:
          BGCALL(result, overlaps, Linestring, g1, Linestring, g2, pnull_value);
          break;
        case Geometry::wkb_multilinestring:
          BGCALL(result, overlaps, Linestring, g1, Multilinestring, g2,
                 pnull_value);
          break;
        default:
          DBUG_ASSERT(false);
          break;
      }
      break;
    }
    case Geometry::wkb_multilinestring: {
      switch (gt2) {
        case Geometry::wkb_linestring:
          BGCALL(result, overlaps, Multilinestring, g1, Linestring, g2,
                 pnull_value);
          break;
        case Geometry::wkb_multilinestring:
          BGCALL(result, overlaps, Multilinestring, g1, Multilinestring, g2,
                 pnull_value);
          break;
        default:
          DBUG_ASSERT(false);
          break;
      }
      break;
    }
    case Geometry::wkb_polygon: {
      switch (gt2) {
        case Geometry::wkb_polygon:
          BGCALL(result, overlaps, Polygon, g1, Polygon, g2, pnull_value);
          break;
        case Geometry::wkb_multipolygon:
          BGCALL(result, overlaps, Polygon, g1, Multipolygon, g2, pnull_value);
          break;
        default:
          DBUG_ASSERT(false);
          break;
      }
      break;
    }
    case Geometry::wkb_multipolygon: {
      switch (gt2) {
        case Geometry::wkb_polygon:
          BGCALL(result, overlaps, Multipolygon, g1, Polygon, g2, pnull_value);
          break;
        case Geometry::wkb_multipolygon:
          BGCALL(result, overlaps, Multipolygon, g1, Multipolygon, g2,
                 pnull_value);
          break;
        default:
          DBUG_ASSERT(false);
          break;
      }
      break;
    }
    default:
      DBUG_ASSERT(false);
      break;
  }

  return result;
}

/**
  Do touches relation check of two geometries.
  Dispatch to specific BG functions according to operation type, and 1st or
  both operand types.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
*/
template <typename Geom_types>
int Item_func_spatial_rel::touches_check(Geometry *g1, Geometry *g2,
                                         bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt1 = g1->get_type();
  Geometry::wkbType gt2 = g2->get_type();

  if ((gt1 == Geometry::wkb_point || gt1 == Geometry::wkb_multipoint) &&
      (gt2 == Geometry::wkb_point || gt2 == Geometry::wkb_multipoint)) {
    /*
      OGC says this is not applicable, and we always return false for
      inapplicable situations.
    */
    return 0;
  }
  /*
    Touches is symetric, and one argument is allowed to be a point/multipoint.
   */
  switch (gt1) {
    case Geometry::wkb_point:
      result = BG_wrap<Geom_types>::point_touches_geometry(g1, g2, pnull_value);
      break;
    case Geometry::wkb_multipoint:
      result =
          BG_wrap<Geom_types>::multipoint_touches_geometry(g1, g2, pnull_value);
      break;
    case Geometry::wkb_linestring:
      result =
          BG_wrap<Geom_types>::linestring_touches_geometry(g1, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      result = BG_wrap<Geom_types>::multilinestring_touches_geometry(
          g1, g2, pnull_value);
      break;
    case Geometry::wkb_polygon:
      result =
          BG_wrap<Geom_types>::polygon_touches_geometry(g1, g2, pnull_value);
      break;
    case Geometry::wkb_multipolygon:
      result = BG_wrap<Geom_types>::multipolygon_touches_geometry(g1, g2,
                                                                  pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }
  return result;
}

/**
  Do crosses relation check of two geometries.
  Dispatch to specific BG functions according to operation type, and 1st or
  both operand types.

  @tparam Geom_types Geometry types definitions.
  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
*/
template <typename Geom_types>
int Item_func_spatial_rel::crosses_check(Geometry *g1, Geometry *g2,
                                         bool *pnull_value) {
  int result = 0;
  Geometry::wkbType gt1 = g1->get_type();
  Geometry::wkbType gt2 = g2->get_type();

  if (gt1 == Geometry::wkb_polygon || gt2 == Geometry::wkb_point ||
      (gt1 == Geometry::wkb_multipolygon || gt2 == Geometry::wkb_multipoint)) {
    /*
      OGC says this is not applicable, and we always return false for
      inapplicable situations.
    */
    return 0;
  }

  if (gt1 == Geometry::wkb_point) {
    result = 0;
    return result;
  }

  switch (gt1) {
    case Geometry::wkb_multipoint:
      result =
          BG_wrap<Geom_types>::multipoint_crosses_geometry(g1, g2, pnull_value);
      break;
    case Geometry::wkb_linestring:
      result =
          BG_wrap<Geom_types>::linestring_crosses_geometry(g1, g2, pnull_value);
      break;
    case Geometry::wkb_multilinestring:
      result = BG_wrap<Geom_types>::multilinestring_crosses_geometry(
          g1, g2, pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }

  return result;
}

/**
  Entry point to call Boost Geometry functions to check geometry relations.
  This function is static so that it can be called without the
  Item_func_spatial_rel object --- we do so to implement a few functionality
  for other classes in this file, e.g. Item_func_spatial_operation::val_str.

  @param g1 First Geometry operand, not a geometry collection.
  @param g2 Second Geometry operand, not a geometry collection.
  @param relchk_type The type of relation check.
  @param[out] pnull_value Returns whether error occurred duirng the computation.
  @return 0 if specified relation doesn't hold for the given operands,
                otherwise returns none 0.
 */
int Item_func_spatial_rel::bg_geo_relation_check(Geometry *g1, Geometry *g2,
                                                 Functype relchk_type,
                                                 bool *pnull_value) {
  int result = 0;

  typedef BG_models<boost::geometry::cs::cartesian> Geom_types;

  /*
    Dispatch calls to all specific type combinations for each relation check
    function.

    Boost.Geometry doesn't have dynamic polymorphism,
    e.g. the above Point, Linestring, and Polygon templates don't have a common
    base class template, so we have to dispatch by types.

    The checking functions should set null_value to true if there is error.
   */

  switch (relchk_type) {
    case SP_CONTAINS_FUNC:
      result = within_check<Geom_types>(g2, g1, pnull_value);
      break;
    case SP_WITHIN_FUNC:
      result = within_check<Geom_types>(g1, g2, pnull_value);
      break;
    case SP_EQUALS_FUNC:
      result = equals_check<Geom_types>(g1, g2, pnull_value);
      break;
    case SP_DISJOINT_FUNC:
      result = disjoint_check<Geom_types>(g1, g2, pnull_value);
      break;
    case SP_INTERSECTS_FUNC:
      result = intersects_check<Geom_types>(g1, g2, pnull_value);
      break;
    case SP_OVERLAPS_FUNC:
      result = overlaps_check<Geom_types>(g1, g2, pnull_value);
      break;
    case SP_TOUCHES_FUNC:
      result = touches_check<Geom_types>(g1, g2, pnull_value);
      break;
    case SP_CROSSES_FUNC:
      result = crosses_check<Geom_types>(g1, g2, pnull_value);
      break;
    default:
      DBUG_ASSERT(false);
      break;
  }

  return result;
}

longlong Item_func_spatial_relation::val_int() {
  DBUG_TRACE;
  DBUG_ASSERT(fixed);

  String tmp_value1;
  String tmp_value2;
  String *res1 = args[0]->val_str(&tmp_value1);
  String *res2 = args[1]->val_str(&tmp_value2);

  if ((null_value =
           (!res1 || args[0]->null_value || !res2 || args[1]->null_value))) {
    DBUG_ASSERT(maybe_null);
    return 0;
  }

  if (res1 == nullptr || res2 == nullptr) {
    DBUG_ASSERT(false);
    my_error(ER_GIS_INVALID_DATA, MYF(0), func_name());
    return error_int();
  }

  const dd::Spatial_reference_system *srs1 = nullptr;
  const dd::Spatial_reference_system *srs2 = nullptr;
  std::unique_ptr<gis::Geometry> g1;
  std::unique_ptr<gis::Geometry> g2;
  std::unique_ptr<dd::cache::Dictionary_client::Auto_releaser> releaser(
      new dd::cache::Dictionary_client::Auto_releaser(
          current_thd->dd_client()));
  if (gis::parse_geometry(current_thd, func_name(), res1, &srs1, &g1) ||
      gis::parse_geometry(current_thd, func_name(), res2, &srs2, &g2)) {
    return error_int();
  }

  gis::srid_t srid1 = srs1 == nullptr ? 0 : srs1->id();
  gis::srid_t srid2 = srs2 == nullptr ? 0 : srs2->id();
  if (srid1 != srid2) {
    my_error(ER_GIS_DIFFERENT_SRIDS, MYF(0), func_name(), srid1, srid2);
    return error_int();
  }

  bool result;
  bool error = eval(srs1, g1.get(), g2.get(), &result, &null_value);

  if (error) return error_int();

  if (null_value) {
    DBUG_ASSERT(maybe_null);
    return 0;
  }

  return result;
}

bool Item_func_st_contains::eval(const dd::Spatial_reference_system *srs,
                                 const gis::Geometry *g1,
                                 const gis::Geometry *g2, bool *result,
                                 bool *null) {
  return gis::within(srs, g2, g1, func_name(), result, null);
}

bool Item_func_st_crosses::eval(const dd::Spatial_reference_system *srs,
                                const gis::Geometry *g1,
                                const gis::Geometry *g2, bool *result,
                                bool *null) {
  return gis::crosses(srs, g1, g2, func_name(), result, null);
}

bool Item_func_st_disjoint::eval(const dd::Spatial_reference_system *srs,
                                 const gis::Geometry *g1,
                                 const gis::Geometry *g2, bool *result,
                                 bool *null) {
  return gis::disjoint(srs, g1, g2, func_name(), result, null);
}

bool Item_func_st_equals::eval(const dd::Spatial_reference_system *srs,
                               const gis::Geometry *g1, const gis::Geometry *g2,
                               bool *result, bool *null) {
  return gis::equals(srs, g1, g2, func_name(), result, null);
}

bool Item_func_st_intersects::eval(const dd::Spatial_reference_system *srs,
                                   const gis::Geometry *g1,
                                   const gis::Geometry *g2, bool *result,
                                   bool *null) {
  return gis::intersects(srs, g1, g2, func_name(), result, null);
}

bool Item_func_mbrcontains::eval(const dd::Spatial_reference_system *srs,
                                 const gis::Geometry *g1,
                                 const gis::Geometry *g2, bool *result,
                                 bool *null) {
  return gis::mbr_within(srs, g2, g1, func_name(), result, null);
}

bool Item_func_mbrcoveredby::eval(const dd::Spatial_reference_system *srs,
                                  const gis::Geometry *g1,
                                  const gis::Geometry *g2, bool *result,
                                  bool *null) {
  return gis::mbr_covered_by(srs, g1, g2, func_name(), result, null);
}

bool Item_func_mbrcovers::eval(const dd::Spatial_reference_system *srs,
                               const gis::Geometry *g1, const gis::Geometry *g2,
                               bool *result, bool *null) {
  return gis::mbr_covered_by(srs, g2, g1, func_name(), result, null);
}

bool Item_func_mbrdisjoint::eval(const dd::Spatial_reference_system *srs,
                                 const gis::Geometry *g1,
                                 const gis::Geometry *g2, bool *result,
                                 bool *null) {
  return gis::mbr_disjoint(srs, g1, g2, func_name(), result, null);
}

bool Item_func_mbrequals::eval(const dd::Spatial_reference_system *srs,
                               const gis::Geometry *g1, const gis::Geometry *g2,
                               bool *result, bool *null) {
  return gis::mbr_equals(srs, g1, g2, func_name(), result, null);
}

bool Item_func_mbrintersects::eval(const dd::Spatial_reference_system *srs,
                                   const gis::Geometry *g1,
                                   const gis::Geometry *g2, bool *result,
                                   bool *null) {
  return gis::mbr_intersects(srs, g1, g2, func_name(), result, null);
}

bool Item_func_mbroverlaps::eval(const dd::Spatial_reference_system *srs,
                                 const gis::Geometry *g1,
                                 const gis::Geometry *g2, bool *result,
                                 bool *null) {
  return gis::mbr_overlaps(srs, g1, g2, func_name(), result, null);
}

bool Item_func_mbrtouches::eval(const dd::Spatial_reference_system *srs,
                                const gis::Geometry *g1,
                                const gis::Geometry *g2, bool *result,
                                bool *null) {
  return gis::mbr_touches(srs, g1, g2, func_name(), result, null);
}

bool Item_func_mbrwithin::eval(const dd::Spatial_reference_system *srs,
                               const gis::Geometry *g1, const gis::Geometry *g2,
                               bool *result, bool *null) {
  return gis::mbr_within(srs, g1, g2, func_name(), result, null);
}

bool Item_func_st_overlaps::eval(const dd::Spatial_reference_system *srs,
                                 const gis::Geometry *g1,
                                 const gis::Geometry *g2, bool *result,
                                 bool *null) {
  return gis::overlaps(srs, g1, g2, func_name(), result, null);
}

bool Item_func_st_touches::eval(const dd::Spatial_reference_system *srs,
                                const gis::Geometry *g1,
                                const gis::Geometry *g2, bool *result,
                                bool *null) {
  return gis::touches(srs, g1, g2, func_name(), result, null);
}

bool Item_func_st_within::eval(const dd::Spatial_reference_system *srs,
                               const gis::Geometry *g1, const gis::Geometry *g2,
                               bool *result, bool *null) {
  return gis::within(srs, g1, g2, func_name(), result, null);
}
