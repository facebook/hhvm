/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__SPATIAL_REFERENCE_SYSTEM_INCLUDED
#define DD__SPATIAL_REFERENCE_SYSTEM_INCLUDED

#include <cstddef>  // std::nullptr_t

#include "my_inttypes.h"
#include "nullable.h"
#include "sql/dd/impl/raw/object_keys.h"  // IWYU pragma: keep
#include "sql/dd/types/entity_object.h"   // dd::Entity_object
#include "sql/gis/geometries.h"           // gis::Coordinate_system
#include "sql/gis/srid.h"                 // gis::srid_t

class THD;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Item_name_key;
class Primary_id_key;
class Spatial_reference_system_impl;
class Void_key;

namespace tables {
class Spatial_reference_systems;
}

///////////////////////////////////////////////////////////////////////////

class Spatial_reference_system : virtual public Entity_object {
 public:
  typedef Spatial_reference_system_impl Impl;
  typedef Spatial_reference_system Cache_partition;
  typedef tables::Spatial_reference_systems DD_table;
  typedef Primary_id_key Id_key;
  typedef Item_name_key Name_key;
  typedef Void_key Aux_key;

  // We need a set of functions to update a preallocated key.
  virtual bool update_id_key(Id_key *key) const {
    return update_id_key(key, id());
  }

  static bool update_id_key(Id_key *key, Object_id id);

  virtual bool update_name_key(Name_key *key) const {
    return update_name_key(key, name());
  }

  static bool update_name_key(Name_key *key, const String_type &name);

  virtual bool update_aux_key(Aux_key *) const { return true; }

 public:
  virtual ~Spatial_reference_system() {}

  /////////////////////////////////////////////////////////////////////////
  // created
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong created(bool convert_time) const = 0;
  virtual void set_created(ulonglong created) = 0;

  /////////////////////////////////////////////////////////////////////////
  // last_altered
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong last_altered(bool convert_time) const = 0;
  virtual void set_last_altered(ulonglong last_altered) = 0;

  /////////////////////////////////////////////////////////////////////////
  // organization
  /////////////////////////////////////////////////////////////////////////

  virtual const Mysql::Nullable<String_type> &organization() const = 0;
  virtual void set_organization(const String_type &organization) = 0;
  virtual void set_organization(std::nullptr_t) = 0;

  /////////////////////////////////////////////////////////////////////////
  // organization_coordsys_id
  /////////////////////////////////////////////////////////////////////////

  virtual const Mysql::Nullable<gis::srid_t> &organization_coordsys_id()
      const = 0;
  virtual void set_organization_coordsys_id(
      gis::srid_t organization_coordsys_id) = 0;
  virtual void set_organization_coordsys_id(std::nullptr_t) = 0;

  /////////////////////////////////////////////////////////////////////////
  // definition
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &definition() const = 0;
  virtual void set_definition(const String_type &definition) = 0;
  virtual gis::Coordinate_system cs_type() const = 0;
  virtual bool is_projected() const = 0;
  virtual bool is_cartesian() const = 0;
  virtual bool is_geographic() const = 0;

  /**
    Check whether an SRS has latitude-longitude axis ordering.

    @retval true the axis order is latitude-longitude
    @retval false the SRS is not geographic, or is geographic longitude-latitude
  */
  virtual bool is_lat_long() const = 0;

  virtual double semi_major_axis() const = 0;
  virtual double semi_minor_axis() const = 0;
  virtual double linear_unit() const = 0;
  virtual double angular_unit() const = 0;
  virtual double prime_meridian() const = 0;
  virtual bool positive_east() const = 0;
  virtual bool positive_north() const = 0;

  /// Checks whether the SRS definition is missing a TOWGS84 clause.
  ///
  /// TOWGS84 clauses are not mandatory. However, in order to transform to/from
  /// an SRS, the SRS must either be WGS 84, be a projection of WGS 84, or have
  /// a TOWGS84 clause in the definition.
  ///
  /// @retval true The SRS is missing a TOWGS84 clause.
  /// @retval false The SRS has or doesn't need a TOWGS84 clause.
  virtual bool missing_towgs84() const = 0;

  /// Converts a coordinate value from the SRS unit to radians.
  ///
  /// The conversion does not take axis direction or meridian shifting into
  /// account.
  ///
  /// @param[in] d Value in the SRS unit.
  ///
  /// @return Value in radians.
  virtual double to_radians(double d) const = 0;

  /// Converts a coordinate value from radians to the SRS unit.
  ///
  /// The conversion does not take axis direction or meridian shifting into
  /// account.
  ///
  /// @param[in] d Value in radians.
  ///
  /// @return Value in the SRS unit.
  virtual double from_radians(double d) const = 0;

  /// Converts a latitude value from the SRS unit and direction to the in-memory
  /// representation of latitude (radians, positive North).
  ///
  /// @param[in] d Latitude in the SRS unit and axis direction.
  ///
  /// @return Latitude in radians, positive North.
  virtual double to_normalized_latitude(double d) const = 0;

  /// Converts a latitude value from the in-memory representation of latitude
  /// (radians, positive North) to the SRS unit and direction.
  ///
  /// @param[in] d Latitude in radians, positive North.
  ///
  /// @return Latitude in the SRS unit and axis direction.
  virtual double from_normalized_latitude(double d) const = 0;

  /// Converts a longitude value from the SRS unit, direction and meridian to
  /// the in-memory representation of longitude (radians, positive East,
  /// Greenwich meridian).
  ///
  /// @param[in] d Longitude in the SRS unit, axis direction and meridian.
  ///
  /// @return Longitude in radians, positive East, Greenwich meridian.
  virtual double to_normalized_longitude(double d) const = 0;

  /// Converts a longitude value from the in-memory representation of longitude
  /// (radians, positive East, Greenwich meridian) to the SRS unit, direction
  /// and meridian.
  ///
  /// @param[in] d Longitude in radians, positive East, Greenwich meridian.
  ///
  /// @return Longitude in the SRS unit, axis direction and meridian.
  virtual double from_normalized_longitude(double d) const = 0;

  /// Checks if this SRS can be changed to another SRS definition without
  /// changing any computations.
  ///
  /// It's not allowed to change any numbers that affect computations, but it is
  /// allowed to add TOWGS84 parameters if the SRS doesn't already have any.
  ///
  /// @param[in] srs The SRS to compare with.
  ///
  /// @retval true The two SRSs are semantically the same.
  /// @retval false The two SRss are not semantically the same, or of types
  /// which comparison rules are unknown (e.g., unknown projection).
  virtual bool can_be_modified_to(
      const Spatial_reference_system &srs) const = 0;

  /// Gets the proj4 parameters for this SRS.
  ///
  /// An empty string is returned in case we don't know how to create proj4
  /// parameters for this SRS.
  ///
  /// @return The proj4 parameter string, or an empty string.
  virtual String_type proj4_parameters() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // description
  /////////////////////////////////////////////////////////////////////////

  virtual const Mysql::Nullable<String_type> &description() const = 0;
  virtual void set_description(const String_type &description) = 0;
  virtual void set_description(std::nullptr_t) = 0;

  /**
    Allocate a new object and invoke the copy constructor

    @return pointer to dynamically allocated copy
  */
  virtual Spatial_reference_system *clone() const = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__SPATIAL_REFERENCE_SYSTEM_INCLUDE
