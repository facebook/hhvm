/*
  Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/**
  @file

  @brief
  This file defines functions to convert exceptions to MySQL error messages.
*/

#include "sql/sql_exception_handler.h"

// boost::geometry::centroid_exception
#include <boost/geometry/algorithms/centroid.hpp>
// boost::geometry::overlay_invalid_input_exception
#include <boost/geometry/algorithms/detail/has_self_intersections.hpp>
// boost::geometry::turn_info_exception
#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>
// boost::geometry::inconsistent_turns_exception
#include <boost/geometry/algorithms/detail/overlay/inconsistent_turns_exception.hpp>
// boost::geometry::detail::self_get_turn_points::self_ip_exception
#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>
// boost::geometry::empty_input_exception
// boost::geometry::exception
#include <boost/geometry/core/exception.hpp>
#include <new>  // std::bad_alloc
#include <regex>
#include <stdexcept>  // Other std exceptions
#include <string>

#include "my_inttypes.h"      // MYF
#include "my_sys.h"           // my_error
#include "mysqld_error.h"     // Error codes
#include "sql/gis/functor.h"  // gis::not_implemented_exception
#include "sql/gis/gc_utils.h"  // gis::invalid_geometry_exception, gis::too_large_polygon_exception

void handle_std_exception(const char *funcname) {
  try {
    throw;
  } catch (const std::bad_alloc &e) {
    my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), e.what(), funcname);
  } catch (const std::domain_error &e) {
    my_error(ER_STD_DOMAIN_ERROR, MYF(0), e.what(), funcname);
  } catch (const std::length_error &e) {
    my_error(ER_STD_LENGTH_ERROR, MYF(0), e.what(), funcname);
  } catch (const std::invalid_argument &e) {
    my_error(ER_STD_INVALID_ARGUMENT, MYF(0), e.what(), funcname);
  } catch (const std::out_of_range &e) {
    my_error(ER_STD_OUT_OF_RANGE_ERROR, MYF(0), e.what(), funcname);
  } catch (const std::overflow_error &e) {
    my_error(ER_STD_OVERFLOW_ERROR, MYF(0), e.what(), funcname);
  } catch (const std::range_error &e) {
    my_error(ER_STD_RANGE_ERROR, MYF(0), e.what(), funcname);
  } catch (const std::underflow_error &e) {
    my_error(ER_STD_UNDERFLOW_ERROR, MYF(0), e.what(), funcname);
  } catch (const std::logic_error &e) {
    my_error(ER_STD_LOGIC_ERROR, MYF(0), e.what(), funcname);
  } catch (const std::regex_error &e) {
    my_error(ER_STD_REGEX_ERROR, MYF(0), e.what(), funcname);
  } catch (const std::runtime_error &e) {
    my_error(ER_STD_RUNTIME_ERROR, MYF(0), e.what(), funcname);
  } catch (const std::exception &e) {
    my_error(ER_STD_UNKNOWN_EXCEPTION, MYF(0), e.what(), funcname);
  } catch (...) {
    my_error(ER_UNKNOWN_ERROR, MYF(0));
  }
}

void handle_gis_exception(const char *funcname) {
  try {
    throw;
  } catch (const gis::longitude_out_of_range_exception &e) {
    my_error(ER_LONGITUDE_OUT_OF_RANGE, MYF(0), e.value, funcname, e.range_min,
             e.range_max);
  } catch (const gis::latitude_out_of_range_exception &e) {
    my_error(ER_LATITUDE_OUT_OF_RANGE, MYF(0), e.value, funcname, e.range_min,
             e.range_max);
  } catch (const gis::not_implemented_exception &e) {
    int er_variant;
    switch (e.srs_type()) {
      default:
        DBUG_ASSERT(false);  // C++11 woes. /* purecov: inspected */
      case gis::not_implemented_exception::kCartesian:
        er_variant = ER_NOT_IMPLEMENTED_FOR_CARTESIAN_SRS;
        break;
      case gis::not_implemented_exception::Srs_type::kGeographic:
        er_variant = ER_NOT_IMPLEMENTED_FOR_GEOGRAPHIC_SRS;
        break;
      case gis::not_implemented_exception::Srs_type::kProjected:
        er_variant = ER_NOT_IMPLEMENTED_FOR_PROJECTED_SRS;
        break;
    }
    my_error(er_variant, MYF(0), funcname, e.typenames());
  } catch (const gis::invalid_geometry_exception &e) {
    my_error(ER_GIS_INVALID_DATA, MYF(0), funcname);
  } catch (const gis::too_large_polygon_exception &e) {
    my_error(ER_POLYGON_TOO_LARGE, MYF(0), funcname);
  } catch (const boost::geometry::centroid_exception &) {
    my_error(ER_BOOST_GEOMETRY_CENTROID_EXCEPTION, MYF(0), funcname);
  } catch (const boost::geometry::overlay_invalid_input_exception &) {
    my_error(ER_BOOST_GEOMETRY_OVERLAY_INVALID_INPUT_EXCEPTION, MYF(0),
             funcname);
  } catch (const boost::geometry::turn_info_exception &) {
    my_error(ER_BOOST_GEOMETRY_TURN_INFO_EXCEPTION, MYF(0), funcname);
  } catch (const boost::geometry::empty_input_exception &) {
    my_error(ER_BOOST_GEOMETRY_EMPTY_INPUT_EXCEPTION, MYF(0), funcname);
  } catch (const boost::geometry::inconsistent_turns_exception &) {
    my_error(ER_BOOST_GEOMETRY_INCONSISTENT_TURNS_EXCEPTION, MYF(0));
  } catch (const boost::geometry::exception &) {
    my_error(ER_BOOST_GEOMETRY_UNKNOWN_EXCEPTION, MYF(0), funcname);
  } catch (const std::exception &) {
    handle_std_exception(funcname);
  } catch (...) {
    my_error(ER_GIS_UNKNOWN_EXCEPTION, MYF(0), funcname);
  }
}
