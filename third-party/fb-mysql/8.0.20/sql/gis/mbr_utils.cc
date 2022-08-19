// Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
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
/// This file implements the mbr_disjoint function.

#include "sql/gis/mbr_utils.h"

#include <cmath>  // std::isnan
#include <exception>

#include <boost/geometry.hpp>

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/box.h"
#include "sql/gis/box_traits.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_cs.h"
#include "sql/gis/geometries_traits.h"
#include "template_utils.h"  // down_cast

namespace bg = boost::geometry;

namespace gis {

bool mbrs_are_equal(Box const &mbr1, Box const &mbr2) {
  DBUG_ASSERT(mbr1.coordinate_system() == mbr2.coordinate_system());
  switch (mbr1.coordinate_system()) {
    case Coordinate_system::kCartesian:
      return bg::equals(*down_cast<const Cartesian_box *>(&mbr1),
                        *down_cast<const Cartesian_box *>(&mbr2));
    case Coordinate_system::kGeographic:
      return bg::equals(*down_cast<const Geographic_box *>(&mbr1),
                        *down_cast<const Geographic_box *>(&mbr2));
  }
  DBUG_ASSERT(false); /* purecov: inspected */
  return false;       /* purecov: inspected */
}

bool mbr_is_empty(Box const &mbr) {
  return std::isnan(mbr.min_corner().x()) && std::isnan(mbr.min_corner().y()) &&
         std::isnan(mbr.max_corner().x()) && std::isnan(mbr.max_corner().y());
}

bool mbr_is_point(Box const &mbr) {
  return mbr.min_corner().x() == mbr.max_corner().x() &&
         mbr.min_corner().y() == mbr.max_corner().y();
}

bool mbr_is_line(Box const &mbr) {
  return (mbr.min_corner().x() == mbr.max_corner().x()) !=
         (mbr.min_corner().y() == mbr.max_corner().y());
}

/// Merges a vector of Cartesian MBRs into one common MBR.
///
/// Since the coordinate system doesn't wrap, the order in which MBRs are
/// expanded doesn't matter.
///
/// @param[in] boxes Vector of MBRs to merge.
/// @param[out] mbr The resulting MBR.
static void merge_mbrs(const std::vector<Cartesian_box> &boxes,
                       Cartesian_box *mbr) {
  if (!boxes.empty()) *mbr = boxes[0];
  for (auto &box : boxes) bg::expand(*mbr, box);
}

/// Merges a vector of geographic MBRs into one common MBR.
///
/// The coordinate system wraps, so the MBRs must be expanded in the correct
/// order to avoid creating an MBR that is larger than necessary.
///
/// If the vector of boxes is empty, the result MBR is unchanged.
///
/// @param[in] boxes Vector of MBRs to merge.
/// @param[out] mbr The resulting MBR.
static void merge_mbrs(const std::vector<Geographic_box> &boxes,
                       Geographic_box *mbr) {
  if (!boxes.empty())
    bg::detail::envelope::envelope_range_of_boxes::apply(boxes, *mbr);
}

/// Computes the envelope of a Cartesian geometry.
///
/// The MBR returned may be a collapsed box.
///
/// @param[in] g The geometry.
/// @param[out] mbr The envelope of g.
static void cartesian_envelope(const Geometry *g, Cartesian_box *mbr) {
  switch (g->type()) {
    case Geometry_type::kPoint:
      bg::envelope(*down_cast<const Cartesian_point *>(g), *mbr);
      break;
    case Geometry_type::kLinestring:
      bg::envelope(*down_cast<const Cartesian_linestring *>(g), *mbr);
      break;
    case Geometry_type::kPolygon:
      bg::envelope(*down_cast<const Cartesian_polygon *>(g), *mbr);
      break;
    case Geometry_type::kGeometrycollection: {
      std::vector<Cartesian_box> boxes;
      Cartesian_box geom_mbr;
      for (auto geom : *down_cast<const Cartesian_geometrycollection *>(g)) {
        switch (geom->type()) {
          case Geometry_type::kPoint:
            bg::envelope(*down_cast<const Cartesian_point *>(geom), geom_mbr);
            break;
          case Geometry_type::kLinestring:
            bg::envelope(*down_cast<const Cartesian_linestring *>(geom),
                         geom_mbr);
            break;
          case Geometry_type::kPolygon:
            bg::envelope(*down_cast<const Cartesian_polygon *>(geom), geom_mbr);
            break;
          case Geometry_type::kGeometrycollection:
            cartesian_envelope(geom, &geom_mbr);
            break;
          case Geometry_type::kMultipoint:
            bg::envelope(*down_cast<const Cartesian_multipoint *>(geom),
                         geom_mbr);
            break;
          case Geometry_type::kMultilinestring:
            bg::envelope(*down_cast<const Cartesian_multilinestring *>(geom),
                         geom_mbr);
            break;
          case Geometry_type::kMultipolygon:
            bg::envelope(*down_cast<const Cartesian_multipolygon *>(geom),
                         geom_mbr);
            break;
          case Geometry_type::kGeometry:
            DBUG_ASSERT(false);
            throw new std::exception();
        }

        // Cartesian_boxxes around empty geometries contain NaN in all
        // coordinates. If
        // passed to bg::expand, the result will be a box with all NaN
        // coordinates. Therefore, we skip empty boxes.
        if (!mbr_is_empty(geom_mbr)) boxes.push_back(geom_mbr);
      }
      merge_mbrs(boxes, mbr);
      break;
    }
    case Geometry_type::kMultipoint:
      bg::envelope(*down_cast<const Cartesian_multipoint *>(g), *mbr);
      break;
    case Geometry_type::kMultilinestring:
      bg::envelope(*down_cast<const Cartesian_multilinestring *>(g), *mbr);
      break;
    case Geometry_type::kMultipolygon:
      bg::envelope(*down_cast<const Cartesian_multipolygon *>(g), *mbr);
      break;
    case Geometry_type::kGeometry:
      DBUG_ASSERT(false);
      throw new std::exception();
      break;
  }
}

/// Computes the envelope of a geographic geometry.
///
/// The MBR returned may be a collapsed box.
///
/// @param[in] g The geometry.
/// @param[in] semi_major Semi-major axis of ellipsoid.
/// @param[in] semi_minor Semi-minor axis of ellipsoid.
/// @param[out] mbr The envelope of g.
static void geographic_envelope(const Geometry *g, double semi_major,
                                double semi_minor, Geographic_box *mbr) {
  bg::strategy::envelope::geographic<bg::strategy::andoyer,
                                     bg::srs::spheroid<double>>
  strategy(bg::srs::spheroid<double>(semi_major, semi_minor));
  switch (g->type()) {
    case Geometry_type::kPoint:
      bg::envelope(*down_cast<const Geographic_point *>(g), *mbr);
      break;
    case Geometry_type::kLinestring:
      bg::envelope(*down_cast<const Geographic_linestring *>(g), *mbr,
                   strategy);
      break;
    case Geometry_type::kPolygon:
      bg::envelope(*down_cast<const Geographic_polygon *>(g), *mbr, strategy);
      break;
    case Geometry_type::kGeometrycollection: {
      std::vector<Geographic_box> boxes;
      Geographic_box geom_mbr;
      for (auto geom : *down_cast<const Geographic_geometrycollection *>(g)) {
        switch (geom->type()) {
          case Geometry_type::kPoint:
            bg::envelope(*down_cast<const Geographic_point *>(geom), geom_mbr);
            break;
          case Geometry_type::kLinestring:
            bg::envelope(*down_cast<const Geographic_linestring *>(geom),
                         geom_mbr, strategy);
            break;
          case Geometry_type::kPolygon:
            bg::envelope(*down_cast<const Geographic_polygon *>(geom), geom_mbr,
                         strategy);
            break;
          case Geometry_type::kGeometrycollection:
            geographic_envelope(geom, semi_major, semi_minor, &geom_mbr);
            break;
          case Geometry_type::kMultipoint:
            bg::envelope(*down_cast<const Geographic_multipoint *>(geom),
                         geom_mbr);
            break;
          case Geometry_type::kMultilinestring:
            bg::envelope(*down_cast<const Geographic_multilinestring *>(geom),
                         geom_mbr, strategy);
            break;
          case Geometry_type::kMultipolygon:
            bg::envelope(*down_cast<const Geographic_multipolygon *>(geom),
                         geom_mbr, strategy);
            break;
          case Geometry_type::kGeometry:
            DBUG_ASSERT(false);
            throw new std::exception();
        }

        // Geographic_boxxes around empty geometries contain NaN in all
        // coordinates. If
        // passed to bg::expand, the result will be a box with all NaN
        // coordinates. Therefore, we skip empty boxes.
        if (!mbr_is_empty(geom_mbr)) boxes.push_back(geom_mbr);
      }
      merge_mbrs(boxes, mbr);
      break;
    }
    case Geometry_type::kMultipoint:
      bg::envelope(*down_cast<const Geographic_multipoint *>(g), *mbr);
      break;
    case Geometry_type::kMultilinestring:
      bg::envelope(*down_cast<const Geographic_multilinestring *>(g), *mbr,
                   strategy);
      break;
    case Geometry_type::kMultipolygon:
      bg::envelope(*down_cast<const Geographic_multipolygon *>(g), *mbr,
                   strategy);
      break;
    case Geometry_type::kGeometry:
      DBUG_ASSERT(false);
      throw new std::exception();
      break;
  }
}

void box_envelope(const Geometry *g, const dd::Spatial_reference_system *srs,
                  Box *mbr) {
  switch (g->coordinate_system()) {
    case Coordinate_system::kCartesian:
      cartesian_envelope(g, down_cast<Cartesian_box *>(mbr));
      break;
    case Coordinate_system::kGeographic:
      geographic_envelope(g, srs ? srs->semi_major_axis() : 0.0,
                          srs ? srs->semi_minor_axis() : 0.0,
                          down_cast<Geographic_box *>(mbr));
      break;
  }
}

}  // namespace gis
