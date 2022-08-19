// Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

/// @file
///
/// This file implements the set of functions that storage engines can call to
/// do geometrical operations.

#include "sql/gis/rtree_support.h"

#include <algorithm>  // std::min, std::max
#include <cmath>      // std::isfinite, std::isinf, std::isnan
#include <limits>

#include <boost/geometry.hpp>

#include "my_byteorder.h"  // doubleget, float8get
#include "my_inttypes.h"   // uchar
#include "sql/current_thd.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/gis/box.h"
#include "sql/gis/box_traits.h"
#include "sql/gis/covered_by_functor.h"
#include "sql/gis/disjoint_functor.h"
#include "sql/gis/equals_functor.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_cs.h"
#include "sql/gis/intersects_functor.h"
#include "sql/gis/mbr_utils.h"
#include "sql/gis/srid.h"
#include "sql/gis/wkb.h"
#include "sql/spatial.h"    // SRID_SIZE
#include "sql/sql_class.h"  // THD
#include "sql/srs_fetcher.h"
#include "template_utils.h"  // pointer_cast

namespace bg = boost::geometry;

dd::Spatial_reference_system *fetch_srs(gis::srid_t srid) {
  const dd::Spatial_reference_system *srs = nullptr;
  dd::cache::Dictionary_client::Auto_releaser m_releaser(
      current_thd->dd_client());
  Srs_fetcher fetcher(current_thd);
  if (srid != 0 && fetcher.acquire(srid, &srs)) return nullptr;

  if (srs)
    return srs->clone();
  else
    return nullptr;
}

bool mbr_contain_cmp(const dd::Spatial_reference_system *srs, rtr_mbr_t *a,
                     rtr_mbr_t *b) {
  DBUG_ASSERT(a->xmin <= a->xmax && a->ymin <= a->ymax);
  DBUG_ASSERT(b->xmin <= b->xmax && b->ymin <= b->ymax);

  bool result = false;
  try {
    gis::Covered_by covered_by(srs ? srs->semi_major_axis() : 0.0,
                               srs ? srs->semi_minor_axis() : 0.0);
    if (srs == nullptr || srs->is_cartesian()) {
      gis::Cartesian_box a_box(gis::Cartesian_point(a->xmin, a->ymin),
                               gis::Cartesian_point(a->xmax, a->ymax));
      gis::Cartesian_box b_box(gis::Cartesian_point(b->xmin, b->ymin),
                               gis::Cartesian_point(b->xmax, b->ymax));
      result = covered_by(&b_box, &a_box);
    } else {
      DBUG_ASSERT(srs->is_geographic());
      gis::Geographic_box a_box(
          gis::Geographic_point(srs->to_radians(a->xmin),
                                srs->to_radians(a->ymin)),
          gis::Geographic_point(srs->to_radians(a->xmax),
                                srs->to_radians(a->ymax)));
      gis::Geographic_box b_box(
          gis::Geographic_point(srs->to_radians(b->xmin),
                                srs->to_radians(b->ymin)),
          gis::Geographic_point(srs->to_radians(b->xmax),
                                srs->to_radians(b->ymax)));
      result = covered_by(&b_box, &a_box);
    }
  } catch (...) {
    DBUG_ASSERT(false); /* purecov: inspected */
  }

  return result;
}

bool mbr_equal_cmp(const dd::Spatial_reference_system *srs, rtr_mbr_t *a,
                   rtr_mbr_t *b) {
  // These points should not have initialized values at this point,
  // which are min == DBL_MAX and max == -DBL_MAX.
  DBUG_ASSERT(a->xmin <= a->xmax && a->ymin <= a->ymax);
  DBUG_ASSERT(b->xmin <= b->xmax && b->ymin <= b->ymax);

  bool result = false;
  try {
    gis::Equals equals(srs ? srs->semi_major_axis() : 0.0,
                       srs ? srs->semi_minor_axis() : 0.0);
    if (srs == nullptr || srs->is_cartesian()) {
      gis::Cartesian_box a_box(gis::Cartesian_point(a->xmin, a->ymin),
                               gis::Cartesian_point(a->xmax, a->ymax));
      gis::Cartesian_box b_box(gis::Cartesian_point(b->xmin, b->ymin),
                               gis::Cartesian_point(b->xmax, b->ymax));
      result = equals(&a_box, &b_box);
    } else {
      DBUG_ASSERT(srs->is_geographic());
      gis::Geographic_box a_box(
          gis::Geographic_point(srs->to_radians(a->xmin),
                                srs->to_radians(a->ymin)),
          gis::Geographic_point(srs->to_radians(a->xmax),
                                srs->to_radians(a->ymax)));
      gis::Geographic_box b_box(
          gis::Geographic_point(srs->to_radians(b->xmin),
                                srs->to_radians(b->ymin)),
          gis::Geographic_point(srs->to_radians(b->xmax),
                                srs->to_radians(b->ymax)));
      result = equals(&a_box, &b_box);
    }
  } catch (...) {
    DBUG_ASSERT(false); /* purecov: inspected */
  }

  return result;
}

bool mbr_intersect_cmp(const dd::Spatial_reference_system *srs, rtr_mbr_t *a,
                       rtr_mbr_t *b) {
  try {
    gis::Intersects intersects(srs ? srs->semi_major_axis() : 0.0,
                               srs ? srs->semi_minor_axis() : 0.0);
    if (srs == nullptr || srs->is_cartesian()) {
      gis::Cartesian_box a_box(gis::Cartesian_point(a->xmin, a->ymin),
                               gis::Cartesian_point(a->xmax, a->ymax));
      gis::Cartesian_box b_box(gis::Cartesian_point(b->xmin, b->ymin),
                               gis::Cartesian_point(b->xmax, b->ymax));
      return intersects(&a_box, &b_box);
    } else {
      DBUG_ASSERT(srs->is_geographic());
      gis::Geographic_box a_box(
          gis::Geographic_point(srs->to_radians(a->xmin),
                                srs->to_radians(a->ymin)),
          gis::Geographic_point(srs->to_radians(a->xmax),
                                srs->to_radians(a->ymax)));
      gis::Geographic_box b_box(
          gis::Geographic_point(srs->to_radians(b->xmin),
                                srs->to_radians(b->ymin)),
          gis::Geographic_point(srs->to_radians(b->xmax),
                                srs->to_radians(b->ymax)));
      return intersects(&a_box, &b_box);
    }
  } catch (...) {
    assert(false); /* purecov: inspected */
  }
  return false; /* purecov: dead code */
}

bool mbr_disjoint_cmp(const dd::Spatial_reference_system *srs, rtr_mbr_t *a,
                      rtr_mbr_t *b) {
  try {
    gis::Disjoint disjoint(srs ? srs->semi_major_axis() : 0.0,
                           srs ? srs->semi_minor_axis() : 0.0);
    if (srs == nullptr || srs->is_cartesian()) {
      gis::Cartesian_box a_box(gis::Cartesian_point(a->xmin, a->ymin),
                               gis::Cartesian_point(a->xmax, a->ymax));
      gis::Cartesian_box b_box(gis::Cartesian_point(b->xmin, b->ymin),
                               gis::Cartesian_point(b->xmax, b->ymax));
      return disjoint(&a_box, &b_box);
    } else {
      DBUG_ASSERT(srs->is_geographic());
      gis::Geographic_box a_box(
          gis::Geographic_point(srs->to_radians(a->xmin),
                                srs->to_radians(a->ymin)),
          gis::Geographic_point(srs->to_radians(a->xmax),
                                srs->to_radians(a->ymax)));
      gis::Geographic_box b_box(
          gis::Geographic_point(srs->to_radians(b->xmin),
                                srs->to_radians(b->ymin)),
          gis::Geographic_point(srs->to_radians(b->xmax),
                                srs->to_radians(b->ymax)));
      return disjoint(&a_box, &b_box);
    }
  } catch (...) {
    assert(false); /* purecov: inspected */
  }
  return false; /* purecov: dead code */
}

bool mbr_within_cmp(const dd::Spatial_reference_system *srs, rtr_mbr_t *a,
                    rtr_mbr_t *b) {
  bool result = false;
  try {
    // If min and max coordinates have been reversed, InnoDB expects the result
    // to be inverse too. But not if a and b have the exact same coordinates.
    bool invert = false;
    if (a->xmin > a->xmax && a->ymin > a->ymax &&
        !(a->xmin == b->xmin && a->ymin == b->ymin && a->xmax == b->xmax &&
          a->ymax == b->ymax)) {
      invert = true;
    }

    // Correct the min and max corners to generate proper boxes.
    double a_xmin = std::min(a->xmin, a->xmax);
    double a_ymin = std::min(a->ymin, a->ymax);
    double a_xmax = std::max(a->xmin, a->xmax);
    double a_ymax = std::max(a->ymin, a->ymax);
    double b_xmin = std::min(b->xmin, b->xmax);
    double b_ymin = std::min(b->ymin, b->ymax);
    double b_xmax = std::max(b->xmin, b->xmax);
    double b_ymax = std::max(b->ymin, b->ymax);

    gis::Covered_by covered_by(srs ? srs->semi_major_axis() : 0.0,
                               srs ? srs->semi_minor_axis() : 0.0);
    if (srs == nullptr || srs->is_cartesian()) {
      gis::Cartesian_box a_box(gis::Cartesian_point(a_xmin, a_ymin),
                               gis::Cartesian_point(a_xmax, a_ymax));
      gis::Cartesian_box b_box(gis::Cartesian_point(b_xmin, b_ymin),
                               gis::Cartesian_point(b_xmax, b_ymax));
      result = covered_by(&a_box, &b_box);
    } else {
      DBUG_ASSERT(srs->is_geographic());
      gis::Geographic_box a_box(gis::Geographic_point(srs->to_radians(a_xmin),
                                                      srs->to_radians(a_ymin)),
                                gis::Geographic_point(srs->to_radians(a_xmax),
                                                      srs->to_radians(a_ymax)));
      gis::Geographic_box b_box(gis::Geographic_point(srs->to_radians(b_xmin),
                                                      srs->to_radians(b_ymin)),
                                gis::Geographic_point(srs->to_radians(b_xmax),
                                                      srs->to_radians(b_ymax)));
      result = covered_by(&a_box, &b_box);
    }
    if (invert) result = !result;
  } catch (...) {
    DBUG_ASSERT(false); /* purecov: inspected */
  }

  return result;
}

void mbr_join(const dd::Spatial_reference_system *srs, double *a,
              const double *b, int n_dim MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(n_dim == 2);

  try {
    if (srs == nullptr || srs->is_cartesian()) {
      gis::Cartesian_box a_box(gis::Cartesian_point(a[0], a[2]),
                               gis::Cartesian_point(a[1], a[3]));
      gis::Cartesian_box b_box(gis::Cartesian_point(b[0], b[2]),
                               gis::Cartesian_point(b[1], b[3]));
      bg::expand(a_box, b_box);
      a[0] = a_box.min_corner().x();
      a[1] = a_box.max_corner().x();
      a[2] = a_box.min_corner().y();
      a[3] = a_box.max_corner().y();
    } else {
      DBUG_ASSERT(srs->is_geographic());
      gis::Geographic_box a_box(
          gis::Geographic_point(srs->to_radians(a[0]), srs->to_radians(a[2])),
          gis::Geographic_point(srs->to_radians(a[1]), srs->to_radians(a[3])));
      gis::Geographic_box b_box(
          gis::Geographic_point(srs->to_radians(b[0]), srs->to_radians(b[2])),
          gis::Geographic_point(srs->to_radians(b[1]), srs->to_radians(b[3])));
      bg::expand(a_box, b_box);
      a[0] = srs->from_radians(a_box.min_corner().x());
      a[1] = srs->from_radians(a_box.max_corner().x());
      a[2] = srs->from_radians(a_box.min_corner().y());
      a[3] = srs->from_radians(a_box.max_corner().y());
    }
  } catch (...) {
    DBUG_ASSERT(false); /* purecov: inspected */
  }
}

double mbr_join_area(const dd::Spatial_reference_system *srs, const double *a,
                     const double *b, int n_dim MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(n_dim == 2);

  double area = 0.0;
  try {
    if (srs == nullptr || srs->is_cartesian()) {
      gis::Cartesian_box a_box(gis::Cartesian_point(a[0], a[2]),
                               gis::Cartesian_point(a[1], a[3]));
      gis::Cartesian_box b_box(gis::Cartesian_point(b[0], b[2]),
                               gis::Cartesian_point(b[1], b[3]));
      bg::expand(a_box, b_box);
      area = bg::area(a_box);
    } else {
      DBUG_ASSERT(srs->is_geographic());
      gis::Geographic_box a_box(
          gis::Geographic_point(srs->to_radians(a[0]), srs->to_radians(a[2])),
          gis::Geographic_point(srs->to_radians(a[1]), srs->to_radians(a[3])));
      gis::Geographic_box b_box(
          gis::Geographic_point(srs->to_radians(b[0]), srs->to_radians(b[2])),
          gis::Geographic_point(srs->to_radians(b[1]), srs->to_radians(b[3])));
      bg::expand(a_box, b_box);
      area = bg::area(
          a_box, bg::strategy::area::geographic<>(bg::srs::spheroid<double>(
                     srs->semi_major_axis(), srs->semi_minor_axis())));
    }
  } catch (...) {
    DBUG_ASSERT(false); /* purecov: inspected */
  }

  if (!std::isfinite(area)) area = std::numeric_limits<double>::max();
  return area;
}

double compute_area(const dd::Spatial_reference_system *srs, const double *a,
                    int n_dim MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(n_dim == 2);

  double area = 0.0;
  try {
    if (srs == nullptr || srs->is_cartesian()) {
      gis::Cartesian_box a_box(gis::Cartesian_point(a[0], a[2]),
                               gis::Cartesian_point(a[1], a[3]));
      area = bg::area(a_box);
    } else {
      DBUG_ASSERT(srs->is_geographic());
      gis::Geographic_box a_box(
          gis::Geographic_point(srs->to_radians(a[0]), srs->to_radians(a[2])),
          gis::Geographic_point(srs->to_radians(a[1]), srs->to_radians(a[3])));
      area = bg::area(
          a_box, bg::strategy::area::geographic<>(bg::srs::spheroid<double>(
                     srs->semi_major_axis(), srs->semi_minor_axis())));
    }
  } catch (...) {
    DBUG_ASSERT(false); /* purecov: inspected */
  }

  return area;
}

int get_mbr_from_store(const dd::Spatial_reference_system *srs,
                       const uchar *store, uint size,
                       uint n_dims MY_ATTRIBUTE((unused)), double *mbr,
                       gis::srid_t *srid) {
  DBUG_ASSERT(n_dims == 2);
  // The SRS should match the SRID of the geometry, with one exception: For
  // backwards compatibility it is allowed to create indexes with mixed
  // SRIDs. Although these indexes can never be used to optimize queries, the
  // user is allowed to create them. These indexes will call get_mbr_from_store
  // with srs == nullptr. There is, unfortunately, no way to differentiate mixed
  // SRID indexes from SRID 0 indexes here, so the assertion is not perfect.
  DBUG_ASSERT(srs == nullptr || (srs->id() == uint4korr(store)));

  if (srid != nullptr) *srid = uint4korr(store);

  try {
    // Note: current_thd may be nullptr here if this function was called from an
    // internal InnoDB thread. In that case, we won't get any stack size check
    // in gis::parse_wkb, but the geometry has been parsed before with the stack
    // size check enabled. We assume we have at least the same amount of stack
    // when called from an internal thread as when called from a MySQL thread.
    std::unique_ptr<gis::Geometry> g =
        gis::parse_wkb(current_thd, srs,
                       pointer_cast<const char *>(store) + sizeof(gis::srid_t),
                       size - sizeof(gis::srid_t), true);
    if (g.get() == nullptr) {
      return -1; /* purecov: inspected */
    }
    if (srs == nullptr || srs->is_cartesian()) {
      gis::Cartesian_box box;
      gis::box_envelope(g.get(), srs, &box);
      mbr[0] = box.min_corner().x();
      mbr[1] = box.max_corner().x();
      mbr[2] = box.min_corner().y();
      mbr[3] = box.max_corner().y();
    } else {
      DBUG_ASSERT(srs->is_geographic());
      gis::Geographic_box box;
      gis::box_envelope(g.get(), srs, &box);
      mbr[0] = srs->from_radians(box.min_corner().x());
      mbr[1] = srs->from_radians(box.max_corner().x());
      mbr[2] = srs->from_radians(box.min_corner().y());
      mbr[3] = srs->from_radians(box.max_corner().y());
    }
  } catch (...) {
    DBUG_ASSERT(false); /* purecov: inspected */
    return -1;
  }

  if (std::isnan(mbr[0])) {
    /* purecov: begin inspected */
    DBUG_ASSERT(std::isnan(mbr[1]) && std::isnan(mbr[2]) && std::isnan(mbr[3]));
    // The geometry is empty, so there is no bounding box. Return a box that
    // covers the entire domain.
    mbr[0] = std::numeric_limits<double>::lowest();
    mbr[1] = std::numeric_limits<double>::max();
    mbr[2] = std::numeric_limits<double>::lowest();
    mbr[3] = std::numeric_limits<double>::max();
    /* purecov: end inspected */
  }

  // xmin <= xmax && ymin <= ymax
  DBUG_ASSERT(mbr[0] <= mbr[1] && mbr[2] <= mbr[3]);

  return 0;
}

double rtree_area_increase(const dd::Spatial_reference_system *srs,
                           const uchar *mbr_a, const uchar *mbr_b,
                           int mbr_len MY_ATTRIBUTE((unused)),
                           double *ab_area) {
  DBUG_ASSERT(mbr_len == sizeof(double) * 4);

  double a_xmin = float8get(mbr_a);
  double a_xmax = float8get(mbr_a + sizeof(double));
  double a_ymin = float8get(mbr_a + sizeof(double) * 2);
  double a_ymax = float8get(mbr_a + sizeof(double) * 3);
  double b_xmin = float8get(mbr_b);
  double b_xmax = float8get(mbr_b + sizeof(double));
  double b_ymin = float8get(mbr_b + sizeof(double) * 2);
  double b_ymax = float8get(mbr_b + sizeof(double) * 3);

  DBUG_ASSERT(a_xmin <= a_xmax && a_ymin <= a_ymax);
  DBUG_ASSERT(b_xmin <= b_xmax && b_ymin <= b_ymax);

  double a_area = 0.0;
  try {
    if (srs == nullptr || srs->is_cartesian()) {
      gis::Cartesian_box a_box(gis::Cartesian_point(a_xmin, a_ymin),
                               gis::Cartesian_point(a_xmax, a_ymax));
      gis::Cartesian_box b_box(gis::Cartesian_point(b_xmin, b_ymin),
                               gis::Cartesian_point(b_xmax, b_ymax));
      a_area = bg::area(a_box);
      if (a_area == 0.0) a_area = 0.001 * 0.001;
      bg::expand(a_box, b_box);
      *ab_area = bg::area(a_box);
    } else {
      DBUG_ASSERT(srs->is_geographic());
      gis::Geographic_box a_box(gis::Geographic_point(srs->to_radians(a_xmin),
                                                      srs->to_radians(a_ymin)),
                                gis::Geographic_point(srs->to_radians(a_xmax),
                                                      srs->to_radians(a_ymax)));
      gis::Geographic_box b_box(gis::Geographic_point(srs->to_radians(b_xmin),
                                                      srs->to_radians(b_ymin)),
                                gis::Geographic_point(srs->to_radians(b_xmax),
                                                      srs->to_radians(b_ymax)));
      a_area = bg::area(
          a_box, bg::strategy::area::geographic<>(bg::srs::spheroid<double>(
                     srs->semi_major_axis(), srs->semi_minor_axis())));
      bg::expand(a_box, b_box);
      *ab_area = bg::area(
          a_box, bg::strategy::area::geographic<>(bg::srs::spheroid<double>(
                     srs->semi_major_axis(), srs->semi_minor_axis())));
    }
    if (std::isinf(a_area)) a_area = std::numeric_limits<double>::max();
    if (std::isinf(*ab_area)) *ab_area = std::numeric_limits<double>::max();
  } catch (...) {
    DBUG_ASSERT(false); /* purecov: inspected */
  }

  DBUG_ASSERT(std::isfinite(*ab_area - a_area));
  return *ab_area - a_area;
}

double rtree_area_overlapping(const dd::Spatial_reference_system *srs,
                              const uchar *mbr_a, const uchar *mbr_b,
                              int mbr_len MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(mbr_len == sizeof(double) * 4);

  double a_xmin = float8get(mbr_a);
  double a_xmax = float8get(mbr_a + sizeof(double));
  double a_ymin = float8get(mbr_a + sizeof(double) * 2);
  double a_ymax = float8get(mbr_a + sizeof(double) * 3);
  double b_xmin = float8get(mbr_b);
  double b_xmax = float8get(mbr_b + sizeof(double));
  double b_ymin = float8get(mbr_b + sizeof(double) * 2);
  double b_ymax = float8get(mbr_b + sizeof(double) * 3);

  DBUG_ASSERT(a_xmin <= a_xmax && a_ymin <= a_ymax);
  DBUG_ASSERT(b_xmin <= b_xmax && b_ymin <= b_ymax);

  double area = 0.0;
  try {
    if (srs == nullptr || srs->is_cartesian()) {
      gis::Cartesian_box a_box(gis::Cartesian_point(a_xmin, a_ymin),
                               gis::Cartesian_point(a_xmax, a_ymax));
      gis::Cartesian_box b_box(gis::Cartesian_point(b_xmin, b_ymin),
                               gis::Cartesian_point(b_xmax, b_ymax));
      gis::Cartesian_box overlapping_box;
      bg::intersection(a_box, b_box, overlapping_box);
      area = bg::area(overlapping_box);
    } else {
      DBUG_ASSERT(srs->is_geographic());
      gis::Geographic_box a_box(gis::Geographic_point(srs->to_radians(a_xmin),
                                                      srs->to_radians(a_ymin)),
                                gis::Geographic_point(srs->to_radians(a_xmax),
                                                      srs->to_radians(a_ymax)));
      gis::Geographic_box b_box(gis::Geographic_point(srs->to_radians(b_xmin),
                                                      srs->to_radians(b_ymin)),
                                gis::Geographic_point(srs->to_radians(b_xmax),
                                                      srs->to_radians(b_ymax)));
      gis::Geographic_box overlapping_box;
      bg::intersection(a_box, b_box, overlapping_box,
                       bg::strategy::intersection::geographic_segments<>(
                           bg::srs::spheroid<double>(srs->semi_major_axis(),
                                                     srs->semi_minor_axis())));
      area =
          bg::area(overlapping_box,
                   bg::strategy::area::geographic<>(bg::srs::spheroid<double>(
                       srs->semi_major_axis(), srs->semi_minor_axis())));
    }
  } catch (...) {
    DBUG_ASSERT(false); /* purecov: inspected */
  }

  if (std::isnan(area)) area = 0.0;
  return area;
}
