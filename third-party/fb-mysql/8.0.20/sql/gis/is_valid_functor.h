#ifndef SQL_GIS_IS_VALID_FUNCTOR_H_INCLUDED
#define SQL_GIS_IS_VALID_FUNCTOR_H_INCLUDED

// Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.
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

#include <boost/geometry.hpp>

#include "sql/gis/functor.h"

namespace gis {

class Is_valid : public Unary_functor<bool> {
 private:
  boost::geometry::strategy::intersection::geographic_segments<>
      m_geographic_ll_la_aa_strategy;

 public:
  Is_valid(double semi_major, double semi_minor);
  virtual bool operator()(const Geometry &g) const;

  bool eval(const Cartesian_point &g) const;
  bool eval(const Cartesian_linestring &g) const;
  bool eval(const Cartesian_polygon &g) const;
  bool eval(const Cartesian_multipoint &g) const;
  bool eval(const Cartesian_multipolygon &g) const;
  bool eval(const Cartesian_multilinestring &g) const;
  bool eval(const Cartesian_geometrycollection &g) const;
  bool eval(const Geographic_point &g) const;
  bool eval(const Geographic_linestring &g) const;
  bool eval(const Geographic_polygon &g) const;
  bool eval(const Geographic_multipoint &g) const;
  bool eval(const Geographic_multipolygon &g) const;
  bool eval(const Geographic_multilinestring &g) const;
  bool eval(const Geographic_geometrycollection &g) const;
};
}  // namespace gis
#endif  // SQL_GIS_IS_VALID_FUNCTOR_H_INCLUDED
