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

/**
  @file
  @brief
  This file contains the implementation for the Item that implements
  ST_Buffer().
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <boost/concept/usage.hpp>
#include <boost/geometry/algorithms/buffer.hpp>
#include <boost/geometry/strategies/agnostic/buffer_distance_symmetric.hpp>
#include <boost/geometry/strategies/buffer.hpp>
#include <boost/geometry/strategies/cartesian/buffer_end_flat.hpp>
#include <boost/geometry/strategies/cartesian/buffer_end_round.hpp>
#include <boost/geometry/strategies/cartesian/buffer_join_miter.hpp>
#include <boost/geometry/strategies/cartesian/buffer_join_round.hpp>
#include <boost/geometry/strategies/cartesian/buffer_point_circle.hpp>
#include <boost/geometry/strategies/cartesian/buffer_point_square.hpp>
#include <boost/geometry/strategies/cartesian/buffer_side_straight.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <memory>  // std::unique_ptr
#include <vector>

#include "m_ctype.h"
#include "m_string.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/derror.h"  // ER_THD
#include "sql/item.h"
#include "sql/item_geofunc.h"
#include "sql/item_geofunc_internal.h"
#include "sql/item_strfunc.h"
#include "sql/parse_tree_node_base.h"
#include "sql/spatial.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_error.h"
#include "sql/sql_exception_handler.h"
#include "sql/srs_fetcher.h"
#include "sql/system_variables.h"
#include "sql_string.h"
#include "template_utils.h"

class PT_item_list;

namespace boost {
namespace geometry {
namespace cs {
struct cartesian;
}  // namespace cs
}  // namespace geometry
}  // namespace boost

static const char *const buffer_strategy_names[] = {
    "invalid_strategy", "end_round",    "end_flat",    "join_round",
    "join_miter",       "point_circle", "point_square"};

template <typename Char_type>
inline int char_icmp(const Char_type a, const Char_type b) {
  const int a1 = std::tolower(a);
  const int b1 = std::tolower(b);
  return a1 > b1 ? 1 : (a1 < b1 ? -1 : 0);
}

/**
  Case insensitive comparison of two ascii strings.
  @param a '\0' ended string.
  @param b '\0' ended string.
 */
template <typename Char_type>
int str_icmp(const Char_type *a, const Char_type *b) {
  int ret = 0, i;

  for (i = 0; a[i] != 0 && b[i] != 0; i++)
    if ((ret = char_icmp(a[i], b[i]))) return ret;
  if (a[i] == 0 && b[i] != 0) return -1;
  if (a[i] != 0 && b[i] == 0) return 1;
  return 0;
}

/*
  Convert strategies stored in String objects into Strategy_setting objects.
*/
void Item_func_buffer::set_strategies() {
  for (int i = 0; i < num_strats; i++) {
    String *pstr = strategies[i];
    const uchar *pstrat = pointer_cast<const uchar *>(pstr->ptr());

    uint32 snum = 0;

    if (pstr->length() != 12 ||
        !((snum = uint4korr(pstrat)) > invalid_strategy &&
          snum <= max_strategy)) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0), "st_buffer");
      null_value = true;
      return;
    }

    const enum_buffer_strategies strat = (enum_buffer_strategies)snum;
    double value = float8get(pstrat + 4);
    enum_buffer_strategy_types strategy_type = invalid_strategy_type;

    switch (strat) {
      case end_round:
      case end_flat:
        strategy_type = end_strategy;
        break;
      case join_round:
      case join_miter:
        strategy_type = join_strategy;
        break;
      case point_circle:
      case point_square:
        strategy_type = point_strategy;
        break;
      default:
        my_error(ER_WRONG_ARGUMENTS, MYF(0), "st_buffer");
        null_value = true;
        return;
        break;
    }

    // Each strategy option can be set no more than once for every ST_Buffer()
    // call.
    if (settings[strategy_type].strategy != invalid_strategy) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0), "st_buffer");
      null_value = true;
      return;
    } else {
      settings[strategy_type].strategy = (enum_buffer_strategies)snum;
      settings[strategy_type].value = value;
    }
  }
}

Item_func_buffer_strategy::Item_func_buffer_strategy(const POS &pos,
                                                     PT_item_list *ilist)
    : Item_str_func(pos, ilist) {
  // Here we want to use the String::set(const char*, ..) version.
  const char *pbuf = tmp_buffer;
  tmp_value.set(pbuf, 0, nullptr);
}

bool Item_func_buffer_strategy::resolve_type(THD *) {
  collation.set(&my_charset_bin);
  decimals = 0;
  max_length = 16;
  maybe_null = true;
  return false;
}

String *Item_func_buffer_strategy::val_str(String * /* str_arg */) {
  String str;
  String *strat_name = args[0]->val_str_ascii(&str);
  if ((null_value = args[0]->null_value)) {
    DBUG_ASSERT(maybe_null);
    return nullptr;
  }

  // Get the NULL-terminated ascii string.
  const char *pstrat_name = strat_name->c_ptr_safe();

  bool found = false;

  tmp_value.set_charset(&my_charset_bin);
  // The tmp_value is supposed to always stores a {uint32,double} pair,
  // and it uses a char tmp_buffer[16] array data member.
  uchar *result_buf = pointer_cast<uchar *>(tmp_value.ptr());

  // Although the result of this item node is never persisted, we still have to
  // use portable endianess access otherwise unaligned access will crash
  // on sparc CPUs.
  for (uint32 i = 0; i <= Item_func_buffer::max_strategy; i++) {
    // The above var_str_ascii() call makes the strat_name an ascii string so
    // we can do below comparison.
    if (str_icmp(pstrat_name, buffer_strategy_names[i]) != 0) continue;

    int4store(result_buf, i);
    result_buf += 4;
    Item_func_buffer::enum_buffer_strategies istrat =
        static_cast<Item_func_buffer::enum_buffer_strategies>(i);

    /*
      The end_flat and point_square strategies must have no more arguments;
      The rest strategies must have 2nd parameter which must be a positive
      numeric value, and we will store it as a double.
      We use float8store to ensure that the value is independent of endianness.
    */
    if (istrat != Item_func_buffer::end_flat &&
        istrat != Item_func_buffer::point_square) {
      if (arg_count != 2) {
        my_error(ER_WRONG_ARGUMENTS, MYF(0), func_name());
        return error_str();
      }

      double val = args[1]->val_real();
      if ((null_value = args[1]->null_value)) {
        DBUG_ASSERT(maybe_null);
        return nullptr;
      }
      if (val <= 0) {
        my_error(ER_WRONG_ARGUMENTS, MYF(0), func_name());
        return error_str();
      }

      if (istrat != Item_func_buffer::join_miter &&
          val > current_thd->variables.max_points_in_geometry) {
        my_error(ER_GIS_MAX_POINTS_IN_GEOMETRY_OVERFLOWED, MYF(0),
                 "points_per_circle",
                 current_thd->variables.max_points_in_geometry, func_name());
        return error_str();
      }

      float8store(result_buf, val);
    } else if (arg_count != 1) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0), func_name());
      return error_str();
    } else
      float8store(result_buf, 0.0);

    found = true;

    break;
  }

  // Unrecognized strategy names, report error.
  if (!found) {
    my_error(ER_WRONG_ARGUMENTS, MYF(0), func_name());
    return error_str();
  }
  tmp_value.length(12);

  return &tmp_value;
}

#define CALL_BG_BUFFER(result, geom, geom_out, dist_strategy, side_strategy, \
                       join_strategy, end_strategy, point_strategy)          \
  do {                                                                       \
    (result) = false;                                                        \
    switch ((geom)->get_type()) {                                            \
      case Geometry::wkb_point: {                                            \
        BG_models<bgcs::cartesian>::Point bg(                                \
            (geom)->get_data_ptr(), (geom)->get_data_size(),                 \
            (geom)->get_flags(), (geom)->get_srid());                        \
        bg::buffer(bg, (geom_out), (dist_strategy), (side_strategy),         \
                   (join_strategy), (end_strategy), (point_strategy));       \
        break;                                                               \
      }                                                                      \
      case Geometry::wkb_multipoint: {                                       \
        BG_models<bgcs::cartesian>::Multipoint bg(                           \
            (geom)->get_data_ptr(), (geom)->get_data_size(),                 \
            (geom)->get_flags(), (geom)->get_srid());                        \
        bg::buffer(bg, (geom_out), (dist_strategy), (side_strategy),         \
                   (join_strategy), (end_strategy), (point_strategy));       \
        break;                                                               \
      }                                                                      \
      case Geometry::wkb_linestring: {                                       \
        BG_models<bgcs::cartesian>::Linestring bg(                           \
            (geom)->get_data_ptr(), (geom)->get_data_size(),                 \
            (geom)->get_flags(), (geom)->get_srid());                        \
        bg::buffer(bg, (geom_out), (dist_strategy), (side_strategy),         \
                   (join_strategy), (end_strategy), (point_strategy));       \
        break;                                                               \
      }                                                                      \
      case Geometry::wkb_multilinestring: {                                  \
        BG_models<bgcs::cartesian>::Multilinestring bg(                      \
            (geom)->get_data_ptr(), (geom)->get_data_size(),                 \
            (geom)->get_flags(), (geom)->get_srid());                        \
        bg::buffer(bg, (geom_out), (dist_strategy), (side_strategy),         \
                   (join_strategy), (end_strategy), (point_strategy));       \
        break;                                                               \
      }                                                                      \
      case Geometry::wkb_polygon: {                                          \
        const void *data_ptr = (geom)->normalize_ring_order();               \
        if (data_ptr == NULL) {                                              \
          my_error(ER_GIS_INVALID_DATA, MYF(0), "st_buffer");                \
          (result) = true;                                                   \
          break;                                                             \
        }                                                                    \
        BG_models<bgcs::cartesian>::Polygon bg(                              \
            data_ptr, (geom)->get_data_size(), (geom)->get_flags(),          \
            (geom)->get_srid());                                             \
        bg::buffer(bg, (geom_out), (dist_strategy), (side_strategy),         \
                   (join_strategy), (end_strategy), (point_strategy));       \
        break;                                                               \
      }                                                                      \
      case Geometry::wkb_multipolygon: {                                     \
        const void *data_ptr = (geom)->normalize_ring_order();               \
        if (data_ptr == NULL) {                                              \
          my_error(ER_GIS_INVALID_DATA, MYF(0), "st_buffer");                \
          (result) = true;                                                   \
          break;                                                             \
        }                                                                    \
        BG_models<bgcs::cartesian>::Multipolygon bg(                         \
            data_ptr, (geom)->get_data_size(), (geom)->get_flags(),          \
            (geom)->get_srid());                                             \
        bg::buffer(bg, (geom_out), (dist_strategy), (side_strategy),         \
                   (join_strategy), (end_strategy), (point_strategy));       \
        break;                                                               \
      }                                                                      \
      default:                                                               \
        DBUG_ASSERT(false);                                                  \
        break;                                                               \
    }                                                                        \
  } while (0)

Item_func_buffer::Item_func_buffer(const POS &pos, PT_item_list *ilist)
    : Item_geometry_func(pos, ilist) {
  num_strats = 0;
  memset(settings, 0, sizeof(settings));
  memset(strategies, 0, sizeof(strategies));
}

namespace bgst = boost::geometry::strategy::buffer;

String *Item_func_buffer::val_str(String *str_value_arg) {
  DBUG_TRACE;
  DBUG_ASSERT(fixed == 1);
  String strat_bufs[side_strategy + 1];

  String *obj = args[0]->val_str(&tmp_value);
  if (!obj || args[0]->null_value) return error_str();

  double dist = args[1]->val_real();
  if (args[1]->null_value) return error_str();

  Geometry_buffer buffer;
  Geometry *geom;
  String *str_result = str_value_arg;

  null_value = false;
  bg_resbuf_mgr.free_result_buffer();

  // Reset the two arrays, set_strategies() requires the settings array to
  // be brand new on every ST_Buffer() call.
  memset(settings, 0, sizeof(settings));
  memset(strategies, 0, sizeof(strategies));

  // Strategies options start from 3rd argument, the 1st two arguments are
  // never strategies: the 1st is input geometry, and the 2nd is distance.
  num_strats = arg_count - 2;
  for (uint i = 2; i < arg_count; i++) {
    strategies[i - 2] = args[i]->val_str(&strat_bufs[i]);
    if (strategies[i - 2] == nullptr || args[i]->null_value) return error_str();
  }

  /*
    Do this before simplify_multi_geometry() in order to exclude invalid
    WKB/WKT data.
   */
  if (!(geom = Geometry::construct(&buffer, obj))) {
    my_error(ER_GIS_INVALID_DATA, MYF(0), func_name());
    return error_str();
  }

  if (geom->get_srid() != 0) {
    THD *thd = current_thd;
    std::unique_ptr<dd::cache::Dictionary_client::Auto_releaser> releaser(
        new dd::cache::Dictionary_client::Auto_releaser(thd->dd_client()));
    Srs_fetcher fetcher(thd);
    const dd::Spatial_reference_system *srs = nullptr;
    if (fetcher.acquire(geom->get_srid(), &srs))
      return error_str();  // Error has already been flagged.

    if (srs == nullptr) {
      my_error(ER_SRS_NOT_FOUND, MYF(0), geom->get_srid());
      return error_str();
    }

    if (!srs->is_cartesian()) {
      DBUG_ASSERT(srs->is_geographic());
      std::string parameters(geom->get_class_info()->m_name.str);
      parameters.append(", ...");
      my_error(ER_NOT_IMPLEMENTED_FOR_GEOGRAPHIC_SRS, MYF(0), func_name(),
               parameters.c_str());
      return error_str();
    }
  }

  /*
    If the input geometry is a multi-geometry or geometry collection that has
    only one component, extract that component as input argument.
  */
  Geometry::wkbType geom_type = geom->get_type();
  if (geom_type == Geometry::wkb_multipoint ||
      geom_type == Geometry::wkb_multipolygon ||
      geom_type == Geometry::wkb_multilinestring ||
      geom_type == Geometry::wkb_geometrycollection) {
    /*
      Make a copy of the geometry byte string argument to work on it,
      don't modify the original one since it is assumed to be stable.
      Simplifying the argument is worth the effort because buffer computation
      is less expensive with simplified geometry.

      The copy's buffer may be directly returned as result so it has to be a
      data member.

      Here we assume that if obj->is_alloced() is false, obj's referring to some
      geometry data stored somewhere else so here we cache the simplified
      version into m_tmp_geombuf without modifying obj's original referred copy;
      otherwise we believe the geometry data
      is solely owned by obj and that each call of this ST_Buffer() is given
      a valid GEOMETRY byte string, i.e. it is structually valid and if it was
      simplified before, the obj->m_length was correctly set to the new length
      after the simplification operation.
     */
    const bool use_buffer = !obj->is_alloced();
    if (simplify_multi_geometry(obj, (use_buffer ? &m_tmp_geombuf : nullptr)) &&
        use_buffer)
      obj = &m_tmp_geombuf;

    if (!(geom = Geometry::construct(&buffer, obj))) {
      my_error(ER_GIS_INVALID_DATA, MYF(0), func_name());
      return error_str();
    }
  }

  /*
    If distance passed to ST_Buffer is too small, then we return the
    original geometry as its buffer. This is needed to avoid division
    overflow in buffer calculation, as well as for performance purposes.
  */
  if (std::abs(dist) <= GIS_ZERO || is_empty_geocollection(geom)) {
    null_value = false;
    str_result = obj;
    return str_result;
  }

  Geometry::wkbType gtype = geom->get_type();
  if (dist < 0 && gtype != Geometry::wkb_polygon &&
      gtype != Geometry::wkb_multipolygon &&
      gtype != Geometry::wkb_geometrycollection) {
    my_error(ER_WRONG_ARGUMENTS, MYF(0), func_name());
    return error_str();
  }

  set_strategies();
  if (null_value) return error_str();

  /*
    str_result will refer to BG object's memory directly if any, here we remove
    last call's remainings so that if this call doesn't produce any result,
    this call won't note down last address(already freed above) and
    next call won't free already free'd memory.
  */
  str_result->set(NullS, 0, &my_charset_bin);
  bool had_except = false;

  try {
    Strategy_setting ss1 = settings[end_strategy];
    Strategy_setting ss2 = settings[join_strategy];
    Strategy_setting ss3 = settings[point_strategy];

    const bool is_pts =
        (gtype == Geometry::wkb_point || gtype == Geometry::wkb_multipoint);

    const bool is_plygn =
        (gtype == Geometry::wkb_polygon || gtype == Geometry::wkb_multipolygon);
    const bool is_ls = (gtype == Geometry::wkb_linestring ||
                        gtype == Geometry::wkb_multilinestring);

    /*
      Some strategies can be applied to only part of the geometry types and
      coordinate systems. For now we only have cartesian coordinate system
      so no check for them.
    */
    if ((is_pts && (ss1.strategy != invalid_strategy ||
                    ss2.strategy != invalid_strategy)) ||
        (is_plygn && (ss1.strategy != invalid_strategy ||
                      ss3.strategy != invalid_strategy)) ||
        (is_ls && ss3.strategy != invalid_strategy)) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0), func_name());
      return error_str();
    }

    bgst::distance_symmetric<double> dist_strat(dist);
    bgst::side_straight side_strat;
    bgst::end_round bgst_end_round(
        ss1.strategy == invalid_strategy ? 32 : ss1.value);
    bgst::end_flat bgst_end_flat;
    bgst::join_round bgst_join_round(
        ss2.strategy == invalid_strategy ? 32 : ss2.value);
    bgst::join_miter bgst_join_miter(ss2.value);
    bgst::point_circle bgst_point_circle(
        ss3.strategy == invalid_strategy ? 32 : ss3.value);
    bgst::point_square bgst_point_square;

    /*
      Default strategies if not specified:
      end_round(32), join_round(32), point_circle(32)
      The order of enum items in enum enum_buffer_strategies is crucial for
      this setting to be correct, don't modify it.

      Although point strategy isn't needed for linear and areal geometries,
      we have to specify it because of bg::buffer interface, and BG will
      silently ignore it. Similarly for other strategies.
    */
    int strats_combination = 0;
    if (ss1.strategy == end_flat) strats_combination |= 1;
    if (ss2.strategy == join_miter) strats_combination |= 2;
    if (ss3.strategy == point_square) strats_combination |= 4;

    BG_models<bgcs::cartesian>::Multipolygon result;
    result.set_srid(geom->get_srid());

    if (geom->get_type() != Geometry::wkb_geometrycollection) {
      bool ret = false;
      switch (strats_combination) {
        case 0:
          CALL_BG_BUFFER(ret, geom, result, dist_strat, side_strat,
                         bgst_join_round, bgst_end_round, bgst_point_circle);
          break;
        case 1:
          CALL_BG_BUFFER(ret, geom, result, dist_strat, side_strat,
                         bgst_join_round, bgst_end_flat, bgst_point_circle);
          break;
        case 2:
          CALL_BG_BUFFER(ret, geom, result, dist_strat, side_strat,
                         bgst_join_miter, bgst_end_round, bgst_point_circle);
          break;
        case 3:
          CALL_BG_BUFFER(ret, geom, result, dist_strat, side_strat,
                         bgst_join_miter, bgst_end_flat, bgst_point_circle);
          break;
        case 4:
          CALL_BG_BUFFER(ret, geom, result, dist_strat, side_strat,
                         bgst_join_round, bgst_end_round, bgst_point_square);
          break;
        case 5:
          CALL_BG_BUFFER(ret, geom, result, dist_strat, side_strat,
                         bgst_join_round, bgst_end_flat, bgst_point_square);
          break;
        case 6:
          CALL_BG_BUFFER(ret, geom, result, dist_strat, side_strat,
                         bgst_join_miter, bgst_end_round, bgst_point_square);
          break;
        case 7:
          CALL_BG_BUFFER(ret, geom, result, dist_strat, side_strat,
                         bgst_join_miter, bgst_end_flat, bgst_point_square);
          break;
        default:
          DBUG_ASSERT(false);
          break;
      }

      if (ret) return error_str();

      if (result.size() == 0) {
        str_result->reserve(GEOM_HEADER_SIZE + 4);
        write_geometry_header(str_result, geom->get_srid(),
                              Geometry::wkb_geometrycollection, 0);
        return str_result;
      } else if (post_fix_result(&bg_resbuf_mgr, result, str_result))
        return error_str();
      bg_resbuf_mgr.set_result_buffer(str_result->ptr());
    } else {
      // Compute buffer for a geometry collection(GC). We first compute buffer
      // for each component of the GC, and put the buffer polygons into another
      // collection, finally merge components of the collection.
      BG_geometry_collection bggc, bggc2;
      bggc.fill(geom);

      for (BG_geometry_collection::Geometry_list::iterator i =
               bggc.get_geometries().begin();
           i != bggc.get_geometries().end(); ++i) {
        BG_models<bgcs::cartesian>::Multipolygon res;
        String temp_result;

        res.set_srid((*i)->get_srid());
        Geometry::wkbType g_type = (*i)->get_type();
        if (dist < 0 && g_type != Geometry::wkb_multipolygon &&
            g_type != Geometry::wkb_polygon) {
          my_error(ER_WRONG_ARGUMENTS, MYF(0), func_name());
          return error_str();
        }

        bool ret = false;
        switch (strats_combination) {
          case 0:
            CALL_BG_BUFFER(ret, *i, res, dist_strat, side_strat,
                           bgst_join_round, bgst_end_round, bgst_point_circle);
            break;
          case 1:
            CALL_BG_BUFFER(ret, *i, res, dist_strat, side_strat,
                           bgst_join_round, bgst_end_flat, bgst_point_circle);
            break;
          case 2:
            CALL_BG_BUFFER(ret, *i, res, dist_strat, side_strat,
                           bgst_join_miter, bgst_end_round, bgst_point_circle);
            break;
          case 3:
            CALL_BG_BUFFER(ret, *i, res, dist_strat, side_strat,
                           bgst_join_miter, bgst_end_flat, bgst_point_circle);
            break;
          case 4:
            CALL_BG_BUFFER(ret, *i, res, dist_strat, side_strat,
                           bgst_join_round, bgst_end_round, bgst_point_square);
            break;
          case 5:
            CALL_BG_BUFFER(ret, *i, res, dist_strat, side_strat,
                           bgst_join_round, bgst_end_flat, bgst_point_square);
            break;
          case 6:
            CALL_BG_BUFFER(ret, *i, res, dist_strat, side_strat,
                           bgst_join_miter, bgst_end_round, bgst_point_square);
            break;
          case 7:
            CALL_BG_BUFFER(ret, *i, res, dist_strat, side_strat,
                           bgst_join_miter, bgst_end_flat, bgst_point_square);
            break;
          default:
            DBUG_ASSERT(false);
            break;
        }

        if (ret) return error_str();
        if (res.size() == 0) continue;
        if (post_fix_result(&bg_resbuf_mgr, res, &temp_result))
          return error_str();

        // A single component's buffer is computed above and stored here.
        bggc2.fill(&res);
      }

      // Merge the accumulated polygons because they may overlap.
      bggc2.merge_components<bgcs::cartesian>(&null_value);
      Gis_geometry_collection *gc = bggc2.as_geometry_collection(str_result);
      delete gc;
    }

    /*
      If the result geometry is a multi-geometry or geometry collection that has
      only one component, extract that component as result.
    */
    simplify_multi_geometry(str_result, nullptr);
  } catch (...) {
    had_except = true;
    handle_gis_exception("st_buffer");
  }

  if (had_except) return error_str();
  return str_result;
}
