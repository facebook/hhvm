// Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.
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

#include "sql/gis/srs/srs.h"

#include <stddef.h>

#include <cmath>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <boost/variant/get.hpp>

#include "m_ctype.h"   // my_strcasecmp
#include "m_string.h"  // my_fcvt_compact
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"  // ER_*
#include "sql/gis/srs/wkt_parser.h"

/// Check that an element doesn't have an authority clause with a different
/// authority name and code.
///
/// The function will return true if the element has no authority clause, or if
/// it has an authority clause with a name that matches (case insensitively)
/// with the name parameter and a code that is the same as the code parameter.
///
/// @tparam T The type of the element with the authority clause.
///
/// @param element The element with the authority clause.
/// @param name The authority name to check for.
/// @param code The authority code to check for.
///
/// @retval true The element has no other authority clause.
/// @retval false The element has another authority clause.
template <typename T>
static bool has_no_conflicting_authority(const T &element, const char *name,
                                         int code) {
  if (!element.authority.valid) return true;
  if (!my_strcasecmp(&my_charset_latin1, name,
                     element.authority.name.c_str())) {
    try {
      int auth_code = std::stoi(element.authority.code);
      if (auth_code == code) return true;  // Authority name and code matches.
    } catch (...) {
      // Code is invalid or out of range.
    }
  }
  return false;
}

/// Check if an element has an authority clause with a given authority name and
/// code.
///
/// The function will return true if the element has an authority clause which
/// name matches (case insensitively) with the name parameter and a code which
/// is the same as the code parameter.
///
/// @tparam T The type of the element with the authority clause.
///
/// @param element The element with the authority clause.
/// @param name The authority name to check for.
/// @param code The authority code to check for.
///
/// @retval true The element has the specified authority clause.
/// @retval false The element doesn't have the specified authority clause.
template <typename T>
static bool has_authority(const T &element, const char *name, int code) {
  if (element.authority.valid &&
      has_no_conflicting_authority(element, name, code))
    return true;
  return false;
}

/**
  Extract projection parameter values from the parse tree and assign
  them to variables.

  The function is given a list of EPSG parameter codes for all
  parameters that can be extracted, and pointers to the variable where
  each parameter should be stored.

  Mandatory parameters must be set to NAN before calling this
  function. Optional parameters must be set to their default value.

  If a mandatory parameter is missing, an error is flagged and the
  function returns true.

  @param[in] srid The spatial reference system ID, used in error reporting
  @param[in] proj Parse tree
  @param[in,out] params List of mandatory parameters (EPSG codes) and
  pointers to where their values should be stored.

  @retval true An error has occurred. The error has been flagged.
  @retval false Success
*/
static bool set_parameters(gis::srid_t srid,
                           gis::srs::wkt_parser::Projected_cs *proj,
                           std::vector<std::pair<int, double *>> *params) {
  std::map<int, std::string> param_names;
  param_names[1026] = "c1";
  param_names[1027] = "c2";
  param_names[1028] = "c3";
  param_names[1029] = "c4";
  param_names[1030] = "c5";
  param_names[1031] = "c6";
  param_names[1032] = "c7";
  param_names[1033] = "c8";
  param_names[1034] = "c9";
  param_names[1035] = "c10";
  param_names[1036] = "azimuth";
  param_names[1038] = "ellipsoid_scale_factor";
  param_names[1039] = "projection_plane_height_at_origin";
  param_names[8617] = "evaluation_point_ordinate_1";
  param_names[8618] = "evaluation_point_ordinate_2";
  param_names[8801] = "latitude_of_origin";
  param_names[8802] = "central_meridian";
  param_names[8805] = "scale_factor";
  param_names[8806] = "false_easting";
  param_names[8807] = "false_northing";
  param_names[8811] = "latitude_of_center";
  param_names[8812] = "longitude_of_center";
  param_names[8813] = "azimuth";
  param_names[8814] = "rectified_grid_angle";
  param_names[8815] = "scale_factor";
  param_names[8816] = "false_easting";
  param_names[8817] = "false_northing";
  param_names[8818] = "pseudo_standard_parallel_1";
  param_names[8819] = "scale_factor";
  param_names[8821] = "latitude_of_origin";
  param_names[8822] = "central_meridian";
  param_names[8823] = "standard_parallel_1";
  param_names[8824] = "standard_parallel_2";
  param_names[8826] = "false_easting";
  param_names[8827] = "false_northing";
  param_names[8830] = "initial_longitude";
  param_names[8831] = "zone_width";
  param_names[8832] = "standard_parallel";
  param_names[8833] = "longitude_of_center";

  std::map<int, std::string> param_aliases;
  param_aliases[8823] = "standard_parallel1";
  param_aliases[8824] = "standard_parallel2";

  /*
    Loop through parameters in the parse tree one by one. For each
    parameter, do this:

    If the parameter has an authority clause with an EPSG code, and
    the authority code matches a code in the list of required
    parameters, assign the value to the parameter variable.

    If there is no authority clause, or the authority is not EPSG,
    check if the name of the parameter matches the name or alias of a
    required parameter. If it does, assign the value to the parameter
    variable.

    Otherwise, ignore the parameter.

    In other words: If a parameter has an EPSG authority code, obey
    it. If not, use the parameter name.
  */
  for (auto i = proj->parameters.begin(); i != proj->parameters.end(); i++) {
    for (size_t j = 0; j < params->size(); j++) {
      if (!my_strcasecmp(&my_charset_latin1, "EPSG",
                         i->authority.name.c_str())) {
        if (!my_strcasecmp(&my_charset_latin1,
                           std::to_string(params->at(j).first).c_str(),
                           i->authority.code.c_str())) {
          *(params->at(j).second) = i->value;
        }
      } else if (!my_strcasecmp(&my_charset_latin1,
                                param_names[params->at(j).first].c_str(),
                                i->name.c_str())) {
        *(params->at(j).second) = i->value;
      } else if (!my_strcasecmp(&my_charset_latin1,
                                param_aliases[params->at(j).first].c_str(),
                                i->name.c_str())) {
        *(params->at(j).second) = i->value;
      }
    }
  }

  // All mandatory parameters are set to NAN before calling this
  // function. If any parameters are still NAN, raise an exception
  // condition.
  for (size_t i = 0; i < params->size(); i++) {
    if (std::isnan(*(params->at(i).second))) {
      int epsg_code = params->at(i).first;
      my_error(ER_SRS_PROJ_PARAMETER_MISSING, MYF(0), srid,
               param_names[epsg_code].c_str(), epsg_code);
      return true;
    }
  }

  return false;
}

static const char *axis_direction_to_name(gis::srs::Axis_direction direction) {
  // Work around bugs in Developer Studio 12.5 on Solaris by casting the enum to
  // int. Otherwise the default case, and only the default case, is always
  // executed. This happens regardless of direction value.
  switch (static_cast<int>(direction)) {
    case static_cast<int>(gis::srs::Axis_direction::UNSPECIFIED):
      return "UNSPECIFIED";
    case static_cast<int>(gis::srs::Axis_direction::NORTH):
      return "NORTH";
    case static_cast<int>(gis::srs::Axis_direction::SOUTH):
      return "SOUTH";
    case static_cast<int>(gis::srs::Axis_direction::EAST):
      return "EAST";
    case static_cast<int>(gis::srs::Axis_direction::WEST):
      return "WEST";
    case static_cast<int>(gis::srs::Axis_direction::OTHER):
      return "OTHER";
    default:
      /* purecov: begin deadcode */
      DBUG_ASSERT(false);
      return "UNKNOWN";
      /* purecov: end */
  }
}

namespace gis {
namespace srs {

bool Geographic_srs::init(gis::srid_t srid,
                          gis::srs::wkt_parser::Geographic_cs *g) {
  // Semi-major axis is required by the parser.
  m_semi_major_axis = g->datum.spheroid.semi_major_axis;
  DBUG_ASSERT(std::isfinite(m_semi_major_axis));
  if (m_semi_major_axis <= 0.0) {
    my_error(ER_SRS_INVALID_SEMI_MAJOR_AXIS, MYF(0));
    return true;
  }

  // Inverse flattening is required by the parser.
  m_inverse_flattening = g->datum.spheroid.inverse_flattening;
  DBUG_ASSERT(std::isfinite(m_inverse_flattening));
  if (m_inverse_flattening != 0.0 && m_inverse_flattening <= 1.0) {
    my_error(ER_SRS_INVALID_INVERSE_FLATTENING, MYF(0));
    return true;
  }

  if (g->datum.towgs84.valid) {
    m_towgs84[0] = g->datum.towgs84.dx;
    m_towgs84[1] = g->datum.towgs84.dy;
    m_towgs84[2] = g->datum.towgs84.dz;
    m_towgs84[3] = g->datum.towgs84.ex;
    m_towgs84[4] = g->datum.towgs84.ey;
    m_towgs84[5] = g->datum.towgs84.ez;
    m_towgs84[6] = g->datum.towgs84.ppm;

    // If not all parameters are used, the parser sets the remaining ones to 0.
    DBUG_ASSERT(std::isfinite(m_towgs84[0]));
    DBUG_ASSERT(std::isfinite(m_towgs84[1]));
    DBUG_ASSERT(std::isfinite(m_towgs84[2]));
    DBUG_ASSERT(std::isfinite(m_towgs84[3]));
    DBUG_ASSERT(std::isfinite(m_towgs84[4]));
    DBUG_ASSERT(std::isfinite(m_towgs84[5]));
    DBUG_ASSERT(std::isfinite(m_towgs84[6]));
  }

  // Angular unit is required by the parser.
  m_angular_unit = g->angular_unit.conversion_factor;
  DBUG_ASSERT(std::isfinite(m_angular_unit));
  if (m_angular_unit <= 0) {
    my_error(ER_SRS_INVALID_ANGULAR_UNIT, MYF(0));
    return true;
  }

  // Prime meridian is required by the parser.
  m_prime_meridian = g->prime_meridian.longitude;
  DBUG_ASSERT(std::isfinite(m_prime_meridian));
  if (m_prime_meridian * m_angular_unit <= -M_PI ||
      m_prime_meridian * m_angular_unit > M_PI) {
    my_error(ER_SRS_INVALID_PRIME_MERIDIAN, MYF(0));
    return true;
  }

  // The parser requires that both axes are specified.
  DBUG_ASSERT(g->axes.valid);
  m_axes[0] = g->axes.x.direction;
  m_axes[1] = g->axes.y.direction;
  if (!(((m_axes[0] == Axis_direction::NORTH ||
          m_axes[0] == Axis_direction::SOUTH) &&
         (m_axes[1] == Axis_direction::EAST ||
          m_axes[1] == Axis_direction::WEST)) ||
        ((m_axes[0] == Axis_direction::EAST ||
          m_axes[0] == Axis_direction::WEST) &&
         (m_axes[1] == Axis_direction::NORTH ||
          m_axes[1] == Axis_direction::SOUTH)))) {
    // Axes are neither lat-long nor long-lat, which doesn't make sense for
    // geographic SRSs.
    my_error(ER_SRS_GEOGCS_INVALID_AXES, MYF(0), srid,
             axis_direction_to_name(m_axes[0]),
             axis_direction_to_name(m_axes[1]));
    return true;
  }

  // Check if this is a valid WGS 84 representation. The requirements are:
  //
  // - The GEOGCS clause must have an authority clause that is EPSG 4326.
  // - All the numbers and axes must match WGS 84 numbers and axes.
  // - There must not be any authority codes in sub-clauses that contradict EPSG
  //   4326.
  //
  // Text strings, apart from authority names, are ignored.
  const double wgs84_semi_major_axis = 6378137.0;
  const double wgs84_inverse_flattening = 298.257223563;
  const double meter = 0.017453292519943278;
  bool wgs84_spheroid =
      has_no_conflicting_authority(g->datum.spheroid, "EPSG", 7030) &&
      m_semi_major_axis == wgs84_semi_major_axis &&
      m_inverse_flattening == wgs84_inverse_flattening;
  bool wgs84_datum =
      has_no_conflicting_authority(g->datum, "EPSG", 6326) && wgs84_spheroid;
  bool wgs84_primem =
      has_no_conflicting_authority(g->prime_meridian, "EPSG", 8901) &&
      m_prime_meridian == 0.0;
  bool wgs84_unit =
      has_no_conflicting_authority(g->angular_unit, "EPSG", 9122) &&
      m_angular_unit == meter;
  bool wgs84_towgs84 =
      !g->datum.towgs84.valid ||
      (m_towgs84[0] == 0.0 && m_towgs84[1] == 0.0 && m_towgs84[2] == 0.0 &&
       m_towgs84[3] == 0.0 && m_towgs84[4] == 0.0 && m_towgs84[5] == 0.0 &&
       m_towgs84[6] == 0.0);
  bool wgs84_axes =
      m_axes[0] == Axis_direction::NORTH && m_axes[1] == Axis_direction::EAST;
  m_is_wgs84 = has_authority(*g, "EPSG", 4326) && wgs84_datum && wgs84_primem &&
               wgs84_unit && wgs84_towgs84 && wgs84_axes;

  return false;
}

bool Geographic_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (srs.srs_type() == Srs_type::GEOGRAPHIC) {
    const Geographic_srs &that = static_cast<const Geographic_srs &>(srs);

    // The SRS is WGS 84 and we're adding a all-zero TOWGS84 clause.
    bool wgs84_add_towgs84 =
        m_is_wgs84 && !has_towgs84() && that.m_towgs84[0] == 0.0 &&
        that.m_towgs84[1] == 0.0 && that.m_towgs84[2] == 0.0 &&
        that.m_towgs84[3] == 0.0 && that.m_towgs84[4] == 0.0 &&
        that.m_towgs84[5] == 0.0 && that.m_towgs84[6] == 0.0;

    // The SRS is WGS 84 and we're removing a TOWGS84 clause. The clause is
    // all-zero and redundant, otherwise m_is_wgs84 would be false.
    bool wgs84_remove_towgs84 = m_is_wgs84 && !that.has_towgs84();

    // The SRS is not WGS 84 and doesn't have a TOWGS84 clause. We're allowed to
    // add a TOWGS84 clause since that doesn't change any currently allowed
    // computations -- it only enables more transformations.
    bool non_wgs84_add_or_no_towgs84 = !m_is_wgs84 && !has_towgs84();

    // Both the current and the new SRS definitions have the same TOWGS84
    // clause.
    bool has_same_towgs84 = has_towgs84() && that.has_towgs84() &&
                            m_towgs84[0] == that.m_towgs84[0] &&
                            m_towgs84[1] == that.m_towgs84[1] &&
                            m_towgs84[2] == that.m_towgs84[2] &&
                            m_towgs84[3] == that.m_towgs84[3] &&
                            m_towgs84[4] == that.m_towgs84[4] &&
                            m_towgs84[5] == that.m_towgs84[5] &&
                            m_towgs84[6] == that.m_towgs84[6];

    return m_semi_major_axis == that.m_semi_major_axis &&
           m_inverse_flattening == that.m_inverse_flattening &&
           (wgs84_add_towgs84 || wgs84_remove_towgs84 ||
            non_wgs84_add_or_no_towgs84 || has_same_towgs84) &&
           m_prime_meridian == that.m_prime_meridian &&
           m_angular_unit == that.m_angular_unit &&
           m_axes[0] == that.m_axes[0] && m_axes[1] == that.m_axes[1] &&
           m_is_wgs84 == that.m_is_wgs84;
  }
  return false;
}

bool Projected_srs::common_proj_parameters_can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (srs.srs_type() == Srs_type::PROJECTED) {
    const Projected_srs &that = static_cast<const Projected_srs &>(srs);
    return m_geographic_srs.can_be_modified_to(that.m_geographic_srs) &&
           m_linear_unit == that.m_linear_unit && m_axes[0] == that.m_axes[0] &&
           m_axes[1] == that.m_axes[1];
  }
  return false;
}

std::string Geographic_srs::proj4_parameters() const {
  char double_str[FLOATING_POINT_BUFFER];
  bool error;
  std::stringstream proj4;

  if (!m_is_wgs84 && !has_towgs84())
    return proj4.str();  // Can't convert if there's no path to WGS 84.

  proj4 << "+proj=lonlat ";

  my_fcvt_compact(semi_major_axis(), double_str, &error);
  if (error) return std::string();
  proj4 << "+a=" << double_str;

  if (inverse_flattening() == 0.0) {
    proj4 << " +b=" << double_str;
  } else {
    my_fcvt_compact(inverse_flattening(), double_str, &error);
    if (error) return std::string();
    proj4 << " +rf=" << double_str;
  }

  // Not setting prime meridian ("+pm="). The in-memory representation always
  // uses Greenwich, which is default in BG.

  // Not setting unit (not supported by proj4). The in-memory representation is
  // always in radians, and BG retrieves the unit from type traits.

  // Not setting axis order and direction (+axis). The in-memory representation
  // is always East-North-up, and BG ignores the parameter.

  proj4 << " +towgs84=";
  if (has_towgs84()) {
    for (int i = 0; i < 7; i++) {
      if (i != 0) proj4 << ",";
      my_fcvt_compact(m_towgs84[i], double_str, &error);
      if (error) return std::string();
      proj4 << double_str;
    }
  } else {
    DBUG_ASSERT(m_is_wgs84);
    proj4 << "0,0,0,0,0,0,0";
  }

  proj4 << " +no_defs";  // Don't set any defaults.

  return proj4.str();
}

bool Projected_srs::init(gis::srid_t srid,
                         gis::srs::wkt_parser::Projected_cs *p) {
  bool res = false;

  res = m_geographic_srs.init(srid, &p->geographic_cs);

  m_linear_unit = p->linear_unit.conversion_factor;

  // Linear unit is required by the parser.
  DBUG_ASSERT(!std::isnan(m_linear_unit));

  if (p->axes.valid) {
    m_axes[0] = p->axes.x.direction;
    m_axes[1] = p->axes.y.direction;

    // The parser requires either both or none to be specified.
    DBUG_ASSERT(m_axes[0] != Axis_direction::UNSPECIFIED);
    DBUG_ASSERT(m_axes[1] != Axis_direction::UNSPECIFIED);
  }

  return res;
}

bool Unknown_projected_srs::init(gis::srid_t srid,
                                 gis::srs::wkt_parser::Projected_cs *p) {
  return Projected_srs::init(srid, p);
}

bool Popular_visualisation_pseudo_mercator_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Popular_visualisation_pseudo_mercator_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::POPULAR_VISUALISATION_PSEUDO_MERCATOR) {
    const Popular_visualisation_pseudo_mercator_srs &that =
        static_cast<const Popular_visualisation_pseudo_mercator_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Lambert_azimuthal_equal_area_spherical_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Lambert_azimuthal_equal_area_spherical_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::LAMBERT_AZIMUTHAL_EQUAL_AREA_SPHERICAL) {
    const Lambert_azimuthal_equal_area_spherical_srs &that =
        static_cast<const Lambert_azimuthal_equal_area_spherical_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Equidistant_cylindrical_srs::init(gis::srid_t srid,
                                       gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8823, &m_standard_parallel_1));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Equidistant_cylindrical_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::EQUIDISTANT_CYLINDRICAL) {
    const Equidistant_cylindrical_srs &that =
        static_cast<const Equidistant_cylindrical_srs &>(srs);
    return m_standard_parallel_1 == that.m_standard_parallel_1 &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Equidistant_cylindrical_spherical_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8823, &m_standard_parallel_1));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Equidistant_cylindrical_spherical_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::EQUIDISTANT_CYLINDRICAL_SPHERICAL) {
    const Equidistant_cylindrical_spherical_srs &that =
        static_cast<const Equidistant_cylindrical_spherical_srs &>(srs);
    return m_standard_parallel_1 == that.m_standard_parallel_1 &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Krovak_north_orientated_srs::init(gis::srid_t srid,
                                       gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8811, &m_latitude_of_center));
  params.push_back(std::make_pair(8833, &m_longitude_of_center));
  params.push_back(std::make_pair(1036, &m_azimuth));
  params.push_back(std::make_pair(8818, &m_pseudo_standard_parallel_1));
  params.push_back(std::make_pair(8819, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Krovak_north_orientated_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::KROVAK_NORTH_ORIENTATED) {
    const Krovak_north_orientated_srs &that =
        static_cast<const Krovak_north_orientated_srs &>(srs);
    return m_latitude_of_center == that.m_latitude_of_center &&
           m_longitude_of_center == that.m_longitude_of_center &&
           m_azimuth == that.m_azimuth &&
           m_pseudo_standard_parallel_1 == that.m_pseudo_standard_parallel_1 &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Krovak_modified_srs::init(gis::srid_t srid,
                               gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8811, &m_latitude_of_center));
  params.push_back(std::make_pair(8833, &m_longitude_of_center));
  params.push_back(std::make_pair(1036, &m_azimuth));
  params.push_back(std::make_pair(8818, &m_pseudo_standard_parallel_1));
  params.push_back(std::make_pair(8819, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));
  params.push_back(std::make_pair(8617, &m_evaluation_point_ordinate_1));
  params.push_back(std::make_pair(8618, &m_evaluation_point_ordinate_2));
  params.push_back(std::make_pair(1026, &m_c1));
  params.push_back(std::make_pair(1027, &m_c2));
  params.push_back(std::make_pair(1028, &m_c3));
  params.push_back(std::make_pair(1029, &m_c4));
  params.push_back(std::make_pair(1030, &m_c5));
  params.push_back(std::make_pair(1031, &m_c6));
  params.push_back(std::make_pair(1032, &m_c7));
  params.push_back(std::make_pair(1033, &m_c8));
  params.push_back(std::make_pair(1034, &m_c9));
  params.push_back(std::make_pair(1035, &m_c10));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Krovak_modified_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::KROVAK_MODIFIED) {
    const Krovak_modified_srs &that =
        static_cast<const Krovak_modified_srs &>(srs);
    return m_latitude_of_center == that.m_latitude_of_center &&
           m_longitude_of_center == that.m_longitude_of_center &&
           m_azimuth == that.m_azimuth &&
           m_pseudo_standard_parallel_1 == that.m_pseudo_standard_parallel_1 &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing &&
           m_evaluation_point_ordinate_1 ==
               that.m_evaluation_point_ordinate_1 &&
           m_evaluation_point_ordinate_2 ==
               that.m_evaluation_point_ordinate_2 &&
           m_c1 == that.m_c1 && m_c2 == that.m_c2 && m_c3 == that.m_c3 &&
           m_c4 == that.m_c4 && m_c5 == that.m_c5 && m_c6 == that.m_c6 &&
           m_c7 == that.m_c7 && m_c8 == that.m_c8 && m_c9 == that.m_c9 &&
           m_c10 == that.m_c10;
  }
  return false;
}

bool Krovak_modified_north_orientated_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8811, &m_latitude_of_center));
  params.push_back(std::make_pair(8833, &m_longitude_of_center));
  params.push_back(std::make_pair(1036, &m_azimuth));
  params.push_back(std::make_pair(8818, &m_pseudo_standard_parallel_1));
  params.push_back(std::make_pair(8819, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));
  params.push_back(std::make_pair(8617, &m_evaluation_point_ordinate_1));
  params.push_back(std::make_pair(8618, &m_evaluation_point_ordinate_2));
  params.push_back(std::make_pair(1026, &m_c1));
  params.push_back(std::make_pair(1027, &m_c2));
  params.push_back(std::make_pair(1028, &m_c3));
  params.push_back(std::make_pair(1029, &m_c4));
  params.push_back(std::make_pair(1030, &m_c5));
  params.push_back(std::make_pair(1031, &m_c6));
  params.push_back(std::make_pair(1032, &m_c7));
  params.push_back(std::make_pair(1033, &m_c8));
  params.push_back(std::make_pair(1034, &m_c9));
  params.push_back(std::make_pair(1035, &m_c10));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Krovak_modified_north_orientated_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::KROVAK_MODIFIED_NORTH_ORIENTATED) {
    const Krovak_modified_north_orientated_srs &that =
        static_cast<const Krovak_modified_north_orientated_srs &>(srs);
    return m_latitude_of_center == that.m_latitude_of_center &&
           m_longitude_of_center == that.m_longitude_of_center &&
           m_azimuth == that.m_azimuth &&
           m_pseudo_standard_parallel_1 == that.m_pseudo_standard_parallel_1 &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing &&
           m_evaluation_point_ordinate_1 ==
               that.m_evaluation_point_ordinate_1 &&
           m_evaluation_point_ordinate_2 ==
               that.m_evaluation_point_ordinate_2 &&
           m_c1 == that.m_c1 && m_c2 == that.m_c2 && m_c3 == that.m_c3 &&
           m_c4 == that.m_c4 && m_c5 == that.m_c5 && m_c6 == that.m_c6 &&
           m_c7 == that.m_c7 && m_c8 == that.m_c8 && m_c9 == that.m_c9 &&
           m_c10 == that.m_c10;
  }
  return false;
}

bool Lambert_conic_conformal_2sp_michigan_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8821, &m_latitude_of_origin));
  params.push_back(std::make_pair(8822, &m_longitude_of_origin));
  params.push_back(std::make_pair(8823, &m_standard_parallel_1));
  params.push_back(std::make_pair(8824, &m_standard_parallel_2));
  params.push_back(std::make_pair(8826, &m_false_easting));
  params.push_back(std::make_pair(8827, &m_false_northing));
  params.push_back(std::make_pair(1038, &m_ellipsoid_scale_factor));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Lambert_conic_conformal_2sp_michigan_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::LAMBERT_CONIC_CONFORMAL_2SP_MICHIGAN) {
    const Lambert_conic_conformal_2sp_michigan_srs &that =
        static_cast<const Lambert_conic_conformal_2sp_michigan_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_standard_parallel_1 == that.m_standard_parallel_1 &&
           m_standard_parallel_2 == that.m_standard_parallel_2 &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing &&
           m_ellipsoid_scale_factor == that.m_ellipsoid_scale_factor;
  }
  return false;
}

bool Colombia_urban_srs::init(gis::srid_t srid,
                              gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));
  params.push_back(std::make_pair(1039, &m_projection_plane_height_at_origin));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Colombia_urban_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::COLOMBIA_URBAN) {
    const Colombia_urban_srs &that =
        static_cast<const Colombia_urban_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing &&
           m_projection_plane_height_at_origin ==
               that.m_projection_plane_height_at_origin;
  }
  return false;
}

bool Lambert_conic_conformal_1sp_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8805, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Lambert_conic_conformal_1sp_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::LAMBERT_CONIC_CONFORMAL_1SP) {
    const Lambert_conic_conformal_1sp_srs &that =
        static_cast<const Lambert_conic_conformal_1sp_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Lambert_conic_conformal_2sp_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8821, &m_latitude_of_origin));
  params.push_back(std::make_pair(8822, &m_longitude_of_origin));
  params.push_back(std::make_pair(8823, &m_standard_parallel_1));
  params.push_back(std::make_pair(8824, &m_standard_parallel_2));
  params.push_back(std::make_pair(8826, &m_false_easting));
  params.push_back(std::make_pair(8827, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Lambert_conic_conformal_2sp_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::LAMBERT_CONIC_CONFORMAL_2SP) {
    const Lambert_conic_conformal_2sp_srs &that =
        static_cast<const Lambert_conic_conformal_2sp_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_standard_parallel_1 == that.m_standard_parallel_1 &&
           m_standard_parallel_2 == that.m_standard_parallel_2 &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Lambert_conic_conformal_2sp_belgium_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8821, &m_latitude_of_origin));
  params.push_back(std::make_pair(8822, &m_longitude_of_origin));
  params.push_back(std::make_pair(8823, &m_standard_parallel_1));
  params.push_back(std::make_pair(8824, &m_standard_parallel_2));
  params.push_back(std::make_pair(8826, &m_false_easting));
  params.push_back(std::make_pair(8827, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Lambert_conic_conformal_2sp_belgium_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::LAMBERT_CONIC_CONFORMAL_2SP_BELGIUM) {
    const Lambert_conic_conformal_2sp_belgium_srs &that =
        static_cast<const Lambert_conic_conformal_2sp_belgium_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_standard_parallel_1 == that.m_standard_parallel_1 &&
           m_standard_parallel_2 == that.m_standard_parallel_2 &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Mercator_variant_a_srs::init(gis::srid_t srid,
                                  gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8805, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Mercator_variant_a_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::MERCATOR_VARIANT_A) {
    const Mercator_variant_a_srs &that =
        static_cast<const Mercator_variant_a_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Mercator_variant_b_srs::init(gis::srid_t srid,
                                  gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8823, &m_standard_parallel_1));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Mercator_variant_b_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::MERCATOR_VARIANT_B) {
    const Mercator_variant_b_srs &that =
        static_cast<const Mercator_variant_b_srs &>(srs);
    return m_standard_parallel_1 == that.m_standard_parallel_1 &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Cassini_soldner_srs::init(gis::srid_t srid,
                               gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Cassini_soldner_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::CASSINI_SOLDNER) {
    const Cassini_soldner_srs &that =
        static_cast<const Cassini_soldner_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Transverse_mercator_srs::init(gis::srid_t srid,
                                   gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8805, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Transverse_mercator_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::TRANSVERSE_MERCATOR) {
    const Transverse_mercator_srs &that =
        static_cast<const Transverse_mercator_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Transverse_mercator_south_orientated_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8805, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Transverse_mercator_south_orientated_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::TRANSVERSE_MERCATOR_SOUTH_ORIENTATED) {
    const Transverse_mercator_south_orientated_srs &that =
        static_cast<const Transverse_mercator_south_orientated_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Oblique_stereographic_srs::init(gis::srid_t srid,
                                     gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8805, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Oblique_stereographic_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::OBLIQUE_STEREOGRAPHIC) {
    const Oblique_stereographic_srs &that =
        static_cast<const Oblique_stereographic_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Polar_stereographic_variant_a_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8805, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Polar_stereographic_variant_a_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::POLAR_STEREOGRAPHIC_VARIANT_A) {
    const Polar_stereographic_variant_a_srs &that =
        static_cast<const Polar_stereographic_variant_a_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool New_zealand_map_grid_srs::init(gis::srid_t srid,
                                    gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool New_zealand_map_grid_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::NEW_ZEALAND_MAP_GRID) {
    const New_zealand_map_grid_srs &that =
        static_cast<const New_zealand_map_grid_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Hotine_oblique_mercator_variant_a_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8811, &m_latitude_of_center));
  params.push_back(std::make_pair(8812, &m_longitude_of_center));
  params.push_back(std::make_pair(8813, &m_azimuth));
  params.push_back(std::make_pair(8814, &m_rectified_grid_angle));
  params.push_back(std::make_pair(8815, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Hotine_oblique_mercator_variant_a_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::HOTINE_OBLIQUE_MERCATOR_VARIANT_A) {
    const Hotine_oblique_mercator_variant_a_srs &that =
        static_cast<const Hotine_oblique_mercator_variant_a_srs &>(srs);
    return m_latitude_of_center == that.m_latitude_of_center &&
           m_longitude_of_center == that.m_longitude_of_center &&
           m_azimuth == that.m_azimuth &&
           m_rectified_grid_angle == that.m_rectified_grid_angle &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Laborde_oblique_mercator_srs::init(gis::srid_t srid,
                                        gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8811, &m_latitude_of_center));
  params.push_back(std::make_pair(8812, &m_longitude_of_center));
  params.push_back(std::make_pair(8813, &m_azimuth));
  params.push_back(std::make_pair(8815, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Laborde_oblique_mercator_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::LABORDE_OBLIQUE_MERCATOR) {
    const Laborde_oblique_mercator_srs &that =
        static_cast<const Laborde_oblique_mercator_srs &>(srs);
    return m_latitude_of_center == that.m_latitude_of_center &&
           m_longitude_of_center == that.m_longitude_of_center &&
           m_azimuth == that.m_azimuth &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Hotine_oblique_mercator_variant_b_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8811, &m_latitude_of_center));
  params.push_back(std::make_pair(8812, &m_longitude_of_center));
  params.push_back(std::make_pair(8813, &m_azimuth));
  params.push_back(std::make_pair(8814, &m_rectified_grid_angle));
  params.push_back(std::make_pair(8815, &m_scale_factor));
  params.push_back(std::make_pair(8816, &m_false_easting));
  params.push_back(std::make_pair(8817, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Hotine_oblique_mercator_variant_b_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::HOTINE_OBLIQUE_MERCATOR_VARIANT_B) {
    const Hotine_oblique_mercator_variant_b_srs &that =
        static_cast<const Hotine_oblique_mercator_variant_b_srs &>(srs);
    return m_latitude_of_center == that.m_latitude_of_center &&
           m_longitude_of_center == that.m_longitude_of_center &&
           m_azimuth == that.m_azimuth &&
           m_rectified_grid_angle == that.m_rectified_grid_angle &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Tunisia_mining_grid_srs::init(gis::srid_t srid,
                                   gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8821, &m_latitude_of_origin));
  params.push_back(std::make_pair(8822, &m_longitude_of_origin));
  params.push_back(std::make_pair(8826, &m_false_easting));
  params.push_back(std::make_pair(8827, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Tunisia_mining_grid_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::TUNISIA_MINING_GRID) {
    const Tunisia_mining_grid_srs &that =
        static_cast<const Tunisia_mining_grid_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Lambert_conic_near_conformal_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8805, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Lambert_conic_near_conformal_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::LAMBERT_CONIC_NEAR_CONFORMAL) {
    const Lambert_conic_near_conformal_srs &that =
        static_cast<const Lambert_conic_near_conformal_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool American_polyconic_srs::init(gis::srid_t srid,
                                  gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool American_polyconic_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::AMERICAN_POLYCONIC) {
    const American_polyconic_srs &that =
        static_cast<const American_polyconic_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Krovak_srs::init(gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8811, &m_latitude_of_center));
  params.push_back(std::make_pair(8833, &m_longitude_of_center));
  params.push_back(std::make_pair(1036, &m_azimuth));
  params.push_back(std::make_pair(8818, &m_pseudo_standard_parallel_1));
  params.push_back(std::make_pair(8819, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Krovak_srs::can_be_modified_to(const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::KROVAK) {
    const Krovak_srs &that = static_cast<const Krovak_srs &>(srs);
    return m_latitude_of_center == that.m_latitude_of_center &&
           m_longitude_of_center == that.m_longitude_of_center &&
           m_azimuth == that.m_azimuth &&
           m_pseudo_standard_parallel_1 == that.m_pseudo_standard_parallel_1 &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Lambert_azimuthal_equal_area_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Lambert_azimuthal_equal_area_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::LAMBERT_AZIMUTHAL_EQUAL_AREA) {
    const Lambert_azimuthal_equal_area_srs &that =
        static_cast<const Lambert_azimuthal_equal_area_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Albers_equal_area_srs::init(gis::srid_t srid,
                                 gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8821, &m_latitude_of_origin));
  params.push_back(std::make_pair(8822, &m_longitude_of_origin));
  params.push_back(std::make_pair(8823, &m_standard_parallel_1));
  params.push_back(std::make_pair(8824, &m_standard_parallel_2));
  params.push_back(std::make_pair(8826, &m_false_easting));
  params.push_back(std::make_pair(8827, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Albers_equal_area_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::ALBERS_EQUAL_AREA) {
    const Albers_equal_area_srs &that =
        static_cast<const Albers_equal_area_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_standard_parallel_1 == that.m_standard_parallel_1 &&
           m_standard_parallel_2 == that.m_standard_parallel_2 &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Transverse_mercator_zoned_grid_system_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8830, &m_initial_longitude));
  params.push_back(std::make_pair(8831, &m_zone_width));
  params.push_back(std::make_pair(8805, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Transverse_mercator_zoned_grid_system_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::TRANSVERSE_MERCATOR_ZONED_GRID_SYSTEM) {
    const Transverse_mercator_zoned_grid_system_srs &that =
        static_cast<const Transverse_mercator_zoned_grid_system_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_initial_longitude == that.m_initial_longitude &&
           m_zone_width == that.m_zone_width &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Lambert_conic_conformal_west_orientated_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8805, &m_scale_factor));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Lambert_conic_conformal_west_orientated_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::LAMBERT_CONIC_CONFORMAL_WEST_ORIENTATED) {
    const Lambert_conic_conformal_west_orientated_srs &that =
        static_cast<const Lambert_conic_conformal_west_orientated_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_scale_factor == that.m_scale_factor &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Bonne_south_orientated_srs::init(gis::srid_t srid,
                                      gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Bonne_south_orientated_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::BONNE_SOUTH_ORIENTATED) {
    const Bonne_south_orientated_srs &that =
        static_cast<const Bonne_south_orientated_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Polar_stereographic_variant_b_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8832, &m_standard_parallel));
  params.push_back(std::make_pair(8833, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Polar_stereographic_variant_b_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::POLAR_STEREOGRAPHIC_VARIANT_B) {
    const Polar_stereographic_variant_b_srs &that =
        static_cast<const Polar_stereographic_variant_b_srs &>(srs);
    return m_standard_parallel == that.m_standard_parallel &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Polar_stereographic_variant_c_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8832, &m_standard_parallel));
  params.push_back(std::make_pair(8833, &m_longitude_of_origin));
  params.push_back(std::make_pair(8826, &m_false_easting));
  params.push_back(std::make_pair(8827, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Polar_stereographic_variant_c_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::POLAR_STEREOGRAPHIC_VARIANT_C) {
    const Polar_stereographic_variant_c_srs &that =
        static_cast<const Polar_stereographic_variant_c_srs &>(srs);
    return m_standard_parallel == that.m_standard_parallel &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Guam_projection_srs::init(gis::srid_t srid,
                               gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Guam_projection_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::GUAM_PROJECTION) {
    const Guam_projection_srs &that =
        static_cast<const Guam_projection_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Modified_azimuthal_equidistant_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Modified_azimuthal_equidistant_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::MODIFIED_AZIMUTHAL_EQUIDISTANT) {
    const Modified_azimuthal_equidistant_srs &that =
        static_cast<const Modified_azimuthal_equidistant_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Hyperbolic_cassini_soldner_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8801, &m_latitude_of_origin));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Hyperbolic_cassini_soldner_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::HYPERBOLIC_CASSINI_SOLDNER) {
    const Hyperbolic_cassini_soldner_srs &that =
        static_cast<const Hyperbolic_cassini_soldner_srs &>(srs);
    return m_latitude_of_origin == that.m_latitude_of_origin &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Lambert_cylindrical_equal_area_spherical_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8823, &m_standard_parallel_1));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Lambert_cylindrical_equal_area_spherical_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::LAMBERT_CYLINDRICAL_EQUAL_AREA_SPHERICAL) {
    const Lambert_cylindrical_equal_area_spherical_srs &that =
        static_cast<const Lambert_cylindrical_equal_area_spherical_srs &>(srs);
    return m_standard_parallel_1 == that.m_standard_parallel_1 &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

bool Lambert_cylindrical_equal_area_srs::init(
    gis::srid_t srid, gis::srs::wkt_parser::Projected_cs *p) {
  std::vector<std::pair<int, double *>> params;

  params.push_back(std::make_pair(8823, &m_standard_parallel_1));
  params.push_back(std::make_pair(8802, &m_longitude_of_origin));
  params.push_back(std::make_pair(8806, &m_false_easting));
  params.push_back(std::make_pair(8807, &m_false_northing));

  bool res = Projected_srs::init(srid, p);
  res |= set_parameters(srid, p, &params);

  return res;
}

bool Lambert_cylindrical_equal_area_srs::can_be_modified_to(
    const Spatial_reference_system &srs) const {
  if (common_proj_parameters_can_be_modified_to(srs) &&
      static_cast<const Projected_srs &>(srs).projection_type() ==
          Projection_type::LAMBERT_CYLINDRICAL_EQUAL_AREA) {
    const Lambert_cylindrical_equal_area_srs &that =
        static_cast<const Lambert_cylindrical_equal_area_srs &>(srs);
    return m_standard_parallel_1 == that.m_standard_parallel_1 &&
           m_longitude_of_origin == that.m_longitude_of_origin &&
           m_false_easting == that.m_false_easting &&
           m_false_northing == that.m_false_northing;
  }
  return false;
}

}  // namespace srs
}  // namespace gis

/**
  Create a geographic SRS description from a parse tree.

  @param[in] srid Spatial reference system ID to use when reporting errors
  @param[in] geog Parser output
  @param[in,out] srs Geographic SRS object allocated by the caller

  @retval true An error has occurred
  @retval false Success
*/
static bool create_geographic_srs(gis::srid_t srid,
                                  gis::srs::wkt_parser::Geographic_cs *geog,
                                  gis::srs::Geographic_srs **srs) {
  *srs = new gis::srs::Geographic_srs();

  return (*srs)->init(srid, geog);
}

/**
  Create a new projected SRS object based on EPSG code.

  When creating a projected SRS object for a projection without an
  EPSG code, code 0 should be used.

  If the EPSG code is 0 or unkown, an Unkown_projected_srs object is
  returned.

  @param epsg_code EPSG coordinate operation method (i.e. projection
                   type) code

  @return New projected SRS object. The caller is responsible for
          deleting it.
*/
static gis::srs::Projected_srs *new_projection(int epsg_code) {
  switch (epsg_code) {
    case 0:
      return new gis::srs::Unknown_projected_srs();
    case 1024:
      return new gis::srs::Popular_visualisation_pseudo_mercator_srs();
    case 1027:
      return new gis::srs::Lambert_azimuthal_equal_area_spherical_srs();
    case 1028:
      return new gis::srs::Equidistant_cylindrical_srs();
    case 1029:
      return new gis::srs::Equidistant_cylindrical_spherical_srs();
    case 1041:
      return new gis::srs::Krovak_north_orientated_srs();
    case 1042:
      return new gis::srs::Krovak_modified_srs();
    case 1043:
      return new gis::srs::Krovak_modified_north_orientated_srs();
    case 1051:
      return new gis::srs::Lambert_conic_conformal_2sp_michigan_srs();
    case 1052:
      return new gis::srs::Colombia_urban_srs();
    case 9801:
      return new gis::srs::Lambert_conic_conformal_1sp_srs();
    case 9802:
      return new gis::srs::Lambert_conic_conformal_2sp_srs();
    case 9803:
      return new gis::srs::Lambert_conic_conformal_2sp_belgium_srs();
    case 9804:
      return new gis::srs::Mercator_variant_a_srs();
    case 9805:
      return new gis::srs::Mercator_variant_b_srs();
    case 9806:
      return new gis::srs::Cassini_soldner_srs();
    case 9807:
      return new gis::srs::Transverse_mercator_srs();
    case 9808:
      return new gis::srs::Transverse_mercator_south_orientated_srs();
    case 9809:
      return new gis::srs::Oblique_stereographic_srs();
    case 9810:
      return new gis::srs::Polar_stereographic_variant_a_srs();
    case 9811:
      return new gis::srs::New_zealand_map_grid_srs();
    case 9812:
      return new gis::srs::Hotine_oblique_mercator_variant_a_srs();
    case 9813:
      return new gis::srs::Laborde_oblique_mercator_srs();
    case 9815:
      return new gis::srs::Hotine_oblique_mercator_variant_b_srs();
    case 9816:
      return new gis::srs::Tunisia_mining_grid_srs();
    case 9817:
      return new gis::srs::Lambert_conic_near_conformal_srs();
    case 9818:
      return new gis::srs::American_polyconic_srs();
    case 9819:
      return new gis::srs::Krovak_srs();
    case 9820:
      return new gis::srs::Lambert_azimuthal_equal_area_srs();
    case 9822:
      return new gis::srs::Albers_equal_area_srs();
    case 9824:
      return new gis::srs::Transverse_mercator_zoned_grid_system_srs();
    case 9826:
      return new gis::srs::Lambert_conic_conformal_west_orientated_srs();
    case 9828:
      return new gis::srs::Bonne_south_orientated_srs();
    case 9829:
      return new gis::srs::Polar_stereographic_variant_b_srs();
    case 9830:
      return new gis::srs::Polar_stereographic_variant_c_srs();
    case 9831:
      return new gis::srs::Guam_projection_srs();
    case 9832:
      return new gis::srs::Modified_azimuthal_equidistant_srs();
    case 9833:
      return new gis::srs::Hyperbolic_cassini_soldner_srs();
    case 9834:
      return new gis::srs::Lambert_cylindrical_equal_area_spherical_srs();
    case 9835:
      return new gis::srs::Lambert_cylindrical_equal_area_srs();
    default:
      return new gis::srs::Unknown_projected_srs();
  }
}

/**
  Create a projected SRS description from a parse tree.

  @param[in] srid Spatial reference system ID to use when reporting errors
  @param[in] proj Parser output
  @param[out] srs A newly allocated projected SRS object

  @retval true An error has occurred
  @retval false Success
*/
static bool create_projected_srs(gis::srid_t srid,
                                 gis::srs::wkt_parser::Projected_cs *proj,
                                 gis::srs::Projected_srs **srs) {
  int epsg_code = 0;
  if (!my_strcasecmp(&my_charset_latin1, "EPSG",
                     proj->projection.authority.name.c_str())) {
    try {
      epsg_code = std::stoi(proj->projection.authority.code);
    } catch (...)  // Invalid or out of range.
    {
      epsg_code = 0;
    }
  }

  *srs = new_projection(epsg_code);

  return (*srs)->init(srid, proj);
}

bool gis::srs::parse_wkt(gis::srid_t srid, const char *begin, const char *end,
                         Spatial_reference_system **result) {
  if (begin == nullptr || begin >= (end - 1)) {
    my_error(ER_SRS_PARSE_ERROR, MYF(0), srid);
    return true;
  }

  namespace wp = gis::srs::wkt_parser;

  wp::Coordinate_system cs;
  bool res = wp::parse_wkt(srid, begin, end, &cs);

  if (!res) {
    if (wp::Projected_cs *proj = boost::get<wp::Projected_cs>(&cs)) {
      Projected_srs *tmp = nullptr;
      res = create_projected_srs(srid, proj, &tmp);
      if (res && tmp != nullptr) {
        delete tmp;
        tmp = nullptr;
      }
      *result = tmp;
    }
    if (wp::Geographic_cs *geog = boost::get<wp::Geographic_cs>(&cs)) {
      Geographic_srs *tmp = nullptr;
      res = create_geographic_srs(srid, geog, &tmp);
      if (res && tmp != nullptr) {
        delete tmp;
        tmp = nullptr;
      }
      *result = tmp;
    }
  }

  return res;
}
