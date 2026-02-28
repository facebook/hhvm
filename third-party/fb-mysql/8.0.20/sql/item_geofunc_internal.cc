/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/item_geofunc_internal.h"

#include <string.h>
#include <algorithm>
#include <boost/concept/usage.hpp>
#include <boost/geometry/algorithms/centroid.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/algorithms/overlaps.hpp>
#include <boost/geometry/core/exception.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/predicates.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <iterator>
#include <memory>

#include "m_ctype.h"
#include "m_string.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/current_thd.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/item_func.h"
#include "sql/mdl.h"
#include "sql/parse_tree_node_base.h"
#include "sql/sql_class.h"  // THD
#include "sql/srs_fetcher.h"
#include "sql/system_variables.h"
#include "sql_string.h"
#include "template_utils.h"

namespace dd {
class Spatial_reference_system;
}  // namespace dd

bool Srs_fetcher::lock(gis::srid_t srid, enum_mdl_type lock_type) {
  DBUG_TRACE;
  DBUG_ASSERT(srid != 0);

  char id_str[11];  // uint32 => max 10 digits + \0
  longlong10_to_str(srid, id_str, 10);

  MDL_request mdl_request;
  mdl_request.init_with_source(MDL_key::SRID, "", id_str, lock_type,
                               MDL_TRANSACTION, __FILE__, __LINE__);
  if (m_thd->mdl_context.acquire_lock_nsec(
          &mdl_request, m_thd->variables.lock_wait_timeout_nsec)) {
    /* purecov: begin inspected */
    // If locking fails, an error has already been flagged.
    return true;
    /* purecov: end */
  }

  return false;
}

bool Srs_fetcher::acquire(gis::srid_t srid,
                          const dd::Spatial_reference_system **srs) {
  if (lock(srid, MDL_SHARED_READ)) return true; /* purecov: inspected */

  if (m_thd->dd_client()->acquire(srid, srs))
    return true; /* purecov: inspected */
  return false;
}

bool Srs_fetcher::acquire_for_modification(gis::srid_t srid,
                                           dd::Spatial_reference_system **srs) {
  if (lock(srid, MDL_EXCLUSIVE)) return true; /* purecov: inspected */

  if (m_thd->dd_client()->acquire_for_modification(srid, srs))
    return true; /* purecov: inspected */
  return false;
}

bool Srs_fetcher::srs_exists(THD *thd, gis::srid_t srid, bool *exists) {
  DBUG_ASSERT(exists);
  std::unique_ptr<dd::cache::Dictionary_client::Auto_releaser> releaser(
      new dd::cache::Dictionary_client::Auto_releaser(thd->dd_client()));
  Srs_fetcher fetcher(thd);
  const dd::Spatial_reference_system *srs = nullptr;
  if (fetcher.acquire(srid, &srs)) return true; /* purecov: inspected */
  *exists = (srs != nullptr);
  return false;
}

/**
  Merge all components as appropriate so that the object contains only
  components that don't overlap.

  @tparam Coordsys Coordinate system type, specified using those defined in
          boost::geometry::cs.
  @param[out] pnull_value takes back null_value set during the operation.
 */
template <typename Coordsys>
void BG_geometry_collection::merge_components(bool *pnull_value) {
  if (is_comp_no_overlapped()) return;

  POS pos;
  Item_func_st_union ifsu(pos, nullptr, nullptr);
  bool do_again = true;
  uint32 last_composition[6] = {0}, num_unchanged_composition = 0;
  size_t last_num_geos = 0;

  /*
    After each merge_one_run call, see whether the two indicators change:
    1. total number of geometry components;
    2. total number of each of the 6 types of geometries

    If they don't change for N*N/4 times, break out of the loop. Here N is
    the total number of geometry components.

    There is the rationale:

    Given a geometry collection, it's likely that one effective merge_one_run
    merges a polygon P and the linestring that crosses it (L) to a
    polygon P'(the same one) and another linestring L', the 2 indicators above
    don't change but the merge is actually done. If we merge P'
    and L' again, they should not be considered cross, but given certain data
    BG somehow believes L` still crosses P` even the P and P` are valid, and
    it will give us a L'' and P'' which is different from L' and P'
    respectively, and L'' is still considered crossing P'',
    hence the loop here never breaks out.

    If the collection has N components, and we have X [multi]linestrings and
    N-X polygons, the number of pairs that can be merged is Y = X * (N-X),
    so the largest Y is N*N/4. If the above 2 indicators stay unchanged more
    than N*N/4 times the loop runs, we believe all possible combinations in
    the collection are enumerated and no effective merge is being done any more.

    Note that the L'' and P'' above is different from L' and P' so we can't
    compare GEOMETRY byte string, and geometric comparison is expensive and may
    still compare unequal and we would still be stuck in the endless loop.
  */
  while (!*pnull_value && do_again) {
    do_again = merge_one_run<Coordsys>(&ifsu, pnull_value);
    if (!*pnull_value && do_again) {
      const size_t num_geos = m_geos.size();
      uint32 composition[6] = {0};

      for (size_t i = 0; i < num_geos; ++i)
        composition[m_geos[i]->get_type() - 1]++;

      if (num_geos != last_num_geos ||
          memcmp(composition, last_composition, sizeof(composition))) {
        memcpy(last_composition, composition, sizeof(composition));
        last_num_geos = num_geos;
        num_unchanged_composition = 0;
      } else
        num_unchanged_composition++;

      if (num_unchanged_composition > (last_num_geos * last_num_geos / 4 + 2))
        break;
    }
  }
}

// Explicit template instantiation
template void BG_geometry_collection::merge_components<
    boost::geometry::cs::cartesian>(bool *);

template <typename Coordsys>
inline bool linestring_overlaps_polygon_outerring(const Gis_line_string &ls,
                                                  const Gis_polygon &plgn) {
  Gis_polygon_ring &oring = plgn.outer();
  Gis_line_string ls2(oring.get_ptr(), oring.get_nbytes(), oring.get_flags(),
                      oring.get_srid());
  return boost::geometry::overlaps(ls, ls2);
}

template <typename Coordsys>
bool linear_areal_intersect_infinite(Geometry *g1, Geometry *g2,
                                     bool *pnull_value) {
  bool res = false;

  /*
    If crosses check succeeds, make sure g2 is a valid [multi]polygon, invalid
    ones can be accepted by BG and the cross check would be considered true,
    we should reject such result and return false in this case.
  */
  if (Item_func_spatial_rel::bg_geo_relation_check(
          g1, g2, Item_func::SP_CROSSES_FUNC, pnull_value) &&
      !*pnull_value) {
    Geometry::wkbType g2_type = g2->get_type();
    if (g2_type == Geometry::wkb_polygon) {
      Gis_polygon plgn(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());
      res = bg::is_valid(plgn);
    } else if (g2_type == Geometry::wkb_multipolygon) {
      Gis_multi_polygon mplgn(g2->get_data_ptr(), g2->get_data_size(),
                              g2->get_flags(), g2->get_srid());
      res = bg::is_valid(mplgn);
    } else
      DBUG_ASSERT(false);

    return res;
  }

  if (*pnull_value) return false;

  if (g1->get_type() == Geometry::wkb_linestring) {
    Gis_line_string ls(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                       g1->get_srid());
    if (g2->get_type() == Geometry::wkb_polygon) {
      Gis_polygon plgn(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());
      res = linestring_overlaps_polygon_outerring<Coordsys>(ls, plgn);
    } else {
      Gis_multi_polygon mplgn(g2->get_data_ptr(), g2->get_data_size(),
                              g2->get_flags(), g2->get_srid());
      for (size_t i = 0; i < mplgn.size(); i++) {
        if (linestring_overlaps_polygon_outerring<Coordsys>(ls, mplgn[i]))
          return true;
      }
    }
  } else {
    Gis_multi_line_string mls(g1->get_data_ptr(), g1->get_data_size(),
                              g1->get_flags(), g1->get_srid());
    if (g2->get_type() == Geometry::wkb_polygon) {
      Gis_polygon plgn(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());
      for (size_t i = 0; i < mls.size(); i++) {
        if (linestring_overlaps_polygon_outerring<Coordsys>(mls[i], plgn))
          return true;
      }
    } else {
      Gis_multi_polygon mplgn(g2->get_data_ptr(), g2->get_data_size(),
                              g2->get_flags(), g2->get_srid());
      for (size_t j = 0; j < mls.size(); j++) {
        for (size_t i = 0; i < mplgn.size(); i++) {
          if (linestring_overlaps_polygon_outerring<Coordsys>(mls[j], mplgn[i]))
            return true;
        }
      }
    }
  }

  return res;
}

/**
  Create this class for exception safety --- destroy the objects referenced
  by the pointers in the set when destroying the container.
 */
template <typename T>
class Pointer_vector : public std::vector<T *> {
  typedef std::vector<T *> parent;

 public:
  ~Pointer_vector() {
    for (typename parent::iterator i = this->begin(); i != this->end(); ++i)
      delete (*i);
  }
};

// A unary predicate to locate a target Geometry object pointer from a sequence.
class Is_target_geometry {
  Geometry *m_target;

 public:
  Is_target_geometry(Geometry *t) : m_target(t) {}

  bool operator()(Geometry *g) { return g == m_target; }
};

class Rtree_entry_compare {
 public:
  Rtree_entry_compare() {}

  bool operator()(const BG_rtree_entry &re1, const BG_rtree_entry &re2) const {
    return re1.second < re2.second;
  }
};

/**
  One run of merging components.

  @tparam Coordsys Coordinate system type, specified using those defined in
          boost::geometry::cs.
  @param ifsu the Item_func_spatial_operation object, we here rely on it to
         do union operation.
  @param[out] pnull_value takes back null_value set during the operation.
  @return whether need another call of this function.
 */
template <typename Coordsys>
bool BG_geometry_collection::merge_one_run(Item_func_st_union *ifsu,
                                           bool *pnull_value) {
  Geometry *gres = nullptr;
  bool has_new = false;
  bool &null_value = *pnull_value;
  Pointer_vector<Geometry> added;
  std::vector<String> added_wkbbufs;

  added.reserve(16);
  added_wkbbufs.reserve(16);

  Rtree_index rtree;
  make_rtree(m_geos, &rtree);
  Rtree_result rtree_result;

  for (Geometry_list::iterator i = m_geos.begin(); i != m_geos.end(); ++i) {
    if (*i == NULL) continue;

    BG_box box;
    make_bg_box(*i, &box);
    if (!is_box_valid(box)) continue;

    rtree_result.clear();
    rtree.query(bgi::intersects(box), std::back_inserter(rtree_result));
    /*
      Normally the rtree should be non-empty because at least there is *i
      itself. But if box has NaN coordinates, the rtree can be empty since
      all coordinate comparisons with NaN numbers are false. also if the
      min corner point have greater coordinates than the max corner point,
      the box isn't valid and the rtree can be empty.
     */
    DBUG_ASSERT(rtree_result.size() != 0);

    // Sort rtree_result by Rtree_entry::second in order to make
    // components in fixed order.
    Rtree_entry_compare rtree_entry_compare;
    std::sort(rtree_result.begin(), rtree_result.end(), rtree_entry_compare);

    // Used to stop the nested loop.
    bool stop_it = false;

    for (Rtree_result::iterator j = rtree_result.begin();
         j != rtree_result.end(); ++j) {
      Geometry *geom2 = m_geos[j->second];
      if (*i == geom2 || geom2 == nullptr) continue;

      // Equals is much easier and faster to check, so check it first.
      if (Item_func_spatial_rel::bg_geo_relation_check(
              geom2, *i, Item_func::SP_EQUALS_FUNC, &null_value) &&
          !null_value) {
        *i = NULL;
        break;
      }

      if (null_value) {
        stop_it = true;
        break;
      }

      if (Item_func_spatial_rel::bg_geo_relation_check(
              *i, geom2, Item_func::SP_WITHIN_FUNC, &null_value) &&
          !null_value) {
        *i = NULL;
        break;
      }

      if (null_value) {
        stop_it = true;
        break;
      }

      if (Item_func_spatial_rel::bg_geo_relation_check(
              geom2, *i, Item_func::SP_WITHIN_FUNC, &null_value) &&
          !null_value) {
        m_geos[j->second] = nullptr;
        continue;
      }

      if (null_value) {
        stop_it = true;
        break;
      }

      /*
        If *i and geom2 is a polygon and a linestring that intersect only
        finite number of points, the union result is the same as the two
        geometries, and we would be stuck in an infinite loop. So we must
        detect and exclude this case. All other argument type combinations
        always will get a geometry different from the two arguments.
      */
      char d11 = (*i)->feature_dimension();
      char d12 = geom2->feature_dimension();
      Geometry *geom_d1 = nullptr;
      Geometry *geom_d2 = nullptr;
      bool is_linear_areal = false;

      if (((d11 == 1 && d12 == 2) || (d12 == 1 && d11 == 2))) {
        geom_d1 = (d11 == 1 ? *i : geom2);
        geom_d2 = (d11 == 2 ? *i : geom2);
        if (d11 != 1) {
          const char tmp_dim = d11;
          d11 = d12;
          d12 = tmp_dim;
        }
        is_linear_areal = true;
      }

      /*
        As said above, if one operand is linear, the other is areal, then we
        only proceed the union of them if they intersect infinite number of
        points, i.e. L crosses A or L touches A's outer ring. Note that if L
        touches some of A's inner rings, L must be crossing A, so not gonna
        check the inner rings.
      */
      if ((!is_linear_areal &&
           Item_func_spatial_rel::bg_geo_relation_check(
               *i, geom2, Item_func::SP_INTERSECTS_FUNC, &null_value) &&
           !null_value) ||
          (is_linear_areal && linear_areal_intersect_infinite<Coordsys>(
                                  geom_d1, geom_d2, &null_value))) {
        String wkbres;

        if (null_value) {
          stop_it = true;
          break;
        }

        gres = ifsu->bg_geo_set_op<Coordsys>(*i, geom2, &wkbres);
        null_value = ifsu->null_value;

        if (null_value) {
          if (gres != nullptr && gres != *i && gres != geom2) delete gres;
          stop_it = true;
          break;
        }

        if (gres != *i) *i = NULL;
        if (gres != geom2) m_geos[j->second] = nullptr;
        if (gres != nullptr && gres != *i && gres != geom2) {
          added.push_back(gres);
          String tmp_wkbbuf;
          added_wkbbufs.push_back(tmp_wkbbuf);
          added_wkbbufs.back().takeover(wkbres);
          has_new = true;
          gres = nullptr;
        }
        /*
          Done with *i, it's either adopted, or removed or merged to a new
          geometry.
         */
        break;
      }  // intersects

      if (null_value) {
        stop_it = true;
        break;
      }

    }  // for (*j)

    if (stop_it) break;

  }  // for (*i)

  // Remove deleted Geometry object pointers, then append new components if any.
  Is_target_geometry pred(nullptr);
  Geometry_list::iterator jj =
      std::remove_if(m_geos.begin(), m_geos.end(), pred);
  m_geos.resize(jj - m_geos.begin());

  for (Pointer_vector<Geometry>::iterator i = added.begin(); i != added.end();
       ++i) {
    /*
      Fill rather than directly use *i for consistent memory management.
      The objects pointed by pointers in added will be automatically destroyed.
     */
    fill(*i);
  }

  // The added and added_wkbbufs arrays are destroyed and the Geometry objects
  // in 'added' are freed, and memory buffers in added_wkbbufs are freed too.
  return has_new;
}

inline void reassemble_geometry(Geometry *g) {
  Geometry::wkbType gtype = g->get_geotype();
  if (gtype == Geometry::wkb_polygon)
    down_cast<Gis_polygon *>(g)->to_wkb_unparsed();
  else if (gtype == Geometry::wkb_multilinestring)
    down_cast<Gis_multi_line_string *>(g)->reassemble();
  else if (gtype == Geometry::wkb_multipolygon)
    down_cast<Gis_multi_polygon *>(g)->reassemble();
}

template <typename BG_geotype>
bool post_fix_result(BG_result_buf_mgr *resbuf_mgr, BG_geotype &geout,
                     String *res) {
  DBUG_ASSERT(geout.has_geom_header_space());
  reassemble_geometry(&geout);

  // Such objects returned by BG never have overlapped components.
  if (geout.get_type() == Geometry::wkb_multilinestring ||
      geout.get_type() == Geometry::wkb_multipolygon)
    geout.set_components_no_overlapped(true);
  if (geout.get_ptr() == nullptr) return true;
  if (res) {
    char *resptr = geout.get_cptr() - GEOM_HEADER_SIZE;
    size_t len = geout.get_nbytes();

    /*
      The resptr buffer is now owned by resbuf_mgr and used by res, resptr
      will be released properly by resbuf_mgr.
     */
    resbuf_mgr->add_buffer(resptr);
    /*
      The memory for the result is owned by a BG_result_buf_mgr,
      so use String::set(char*, size_t, const CHARSET_INFO)
      which points the internall buffer to the input argument,
      and sets m_is_alloced = false, signifying the String object
      does not own the buffer.
    */
    res->set(resptr, len + GEOM_HEADER_SIZE, &my_charset_bin);

    // Prefix the GEOMETRY header.
    write_geometry_header(resptr, geout.get_srid(), geout.get_geotype());

    /*
      Give up ownership because the buffer may have to live longer than
      the object.
    */
    geout.set_ownmem(false);
  }

  return false;
}

// Explicit template instantiation
template bool post_fix_result<Gis_line_string>(BG_result_buf_mgr *,
                                               Gis_line_string &, String *);
template bool post_fix_result<Gis_multi_line_string>(BG_result_buf_mgr *,
                                                     Gis_multi_line_string &,
                                                     String *);
template bool post_fix_result<Gis_multi_point>(BG_result_buf_mgr *,
                                               Gis_multi_point &, String *);
template bool post_fix_result<Gis_multi_polygon>(BG_result_buf_mgr *,
                                                 Gis_multi_polygon &, String *);
template bool post_fix_result<Gis_point>(BG_result_buf_mgr *, Gis_point &,
                                         String *);
template bool post_fix_result<Gis_polygon>(BG_result_buf_mgr *, Gis_polygon &,
                                           String *);

class Is_empty_geometry : public WKB_scanner_event_handler {
 public:
  bool is_empty;

  Is_empty_geometry() : is_empty(true) {}

  virtual void on_wkb_start(Geometry::wkbByteOrder, Geometry::wkbType geotype,
                            const void *, uint32, bool) {
    if (is_empty && geotype != Geometry::wkb_geometrycollection)
      is_empty = false;
  }

  virtual void on_wkb_end(const void *) {}

  virtual bool continue_scan() const { return is_empty; }
};

bool is_empty_geocollection(const Geometry *g) {
  if (g->get_geotype() != Geometry::wkb_geometrycollection) return false;

  uint32 num = uint4korr(g->get_cptr());
  if (num == 0) return true;

  Is_empty_geometry checker;
  uint32 len = g->get_data_size();
  wkb_scanner(current_thd, g->get_cptr(), &len,
              Geometry::wkb_geometrycollection, false, &checker);
  return checker.is_empty;
}

bool is_empty_geocollection(const String &wkbres) {
  if (wkbres.ptr() == nullptr) return true;

  uint32 geotype = uint4korr(wkbres.ptr() + SRID_SIZE + 1);

  if (geotype != static_cast<uint32>(Geometry::wkb_geometrycollection))
    return false;

  if (uint4korr(wkbres.ptr() + SRID_SIZE + WKB_HEADER_SIZE) == 0) return true;

  Is_empty_geometry checker;
  uint32 len = static_cast<uint32>(wkbres.length()) - GEOM_HEADER_SIZE;
  wkb_scanner(current_thd, wkbres.ptr() + GEOM_HEADER_SIZE, &len,
              Geometry::wkb_geometrycollection, false, &checker);
  return checker.is_empty;
}
