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

#include "sql/gis/srs/wkt_parser.h"

#include <cctype>

#include <boost/concept/usage.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/iterator/deref.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/comparison/not_equal.hpp>
#include <boost/preprocessor/control/expr_iif.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/logical/bool.hpp>
#include <boost/preprocessor/repetition/for.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/to_seq.hpp>
#include <boost/preprocessor/variadic/elem.hpp>
#include <boost/proto/operators.hpp>
#include <boost/spirit/include/qi.hpp>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"  // ER_*
#include "sql/gis/srs/srs.h"

BOOST_FUSION_ADAPT_STRUCT(gis::srs::wkt_parser::Authority,
                          (bool, valid)(gis::srs::wkt_parser::String,
                                        name)(gis::srs::wkt_parser::String,
                                              code))

BOOST_FUSION_ADAPT_STRUCT(gis::srs::wkt_parser::Spheroid,
                          (gis::srs::wkt_parser::String,
                           name)(double, semi_major_axis)(double,
                                                          inverse_flattening)(
                              gis::srs::wkt_parser::Authority, authority))

BOOST_FUSION_ADAPT_STRUCT(gis::srs::wkt_parser::Towgs84,
                          (bool, valid)(double, dx)(double, dy)(double, dz)(
                              double, ex)(double, ey)(double, ez)(double, ppm))

BOOST_FUSION_ADAPT_STRUCT(
    gis::srs::wkt_parser::Datum,
    (gis::srs::wkt_parser::String,
     name)(gis::srs::wkt_parser::Spheroid,
           spheroid)(gis::srs::wkt_parser::Towgs84,
                     towgs84)(gis::srs::wkt_parser::Authority, authority))

BOOST_FUSION_ADAPT_STRUCT(gis::srs::wkt_parser::Prime_meridian,
                          (gis::srs::wkt_parser::String,
                           name)(double,
                                 longitude)(gis::srs::wkt_parser::Authority,
                                            authority))

BOOST_FUSION_ADAPT_STRUCT(gis::srs::wkt_parser::Unit,
                          (gis::srs::wkt_parser::String,
                           name)(double, conversion_factor)(
                              gis::srs::wkt_parser::Authority, authority))

BOOST_FUSION_ADAPT_STRUCT(gis::srs::wkt_parser::Axis,
                          (gis::srs::wkt_parser::String,
                           name)(gis::srs::Axis_direction, direction))

BOOST_FUSION_ADAPT_STRUCT(gis::srs::wkt_parser::Twin_axes,
                          (bool, valid)(gis::srs::wkt_parser::Axis,
                                        x)(gis::srs::wkt_parser::Axis, y))

BOOST_FUSION_ADAPT_STRUCT(
    gis::srs::wkt_parser::Geographic_cs,
    (gis::srs::wkt_parser::String, name)(gis::srs::wkt_parser::Datum, datum)(
        gis::srs::wkt_parser::Prime_meridian,
        prime_meridian)(gis::srs::wkt_parser::Unit,
                        angular_unit)(gis::srs::wkt_parser::Twin_axes,
                                      axes)(gis::srs::wkt_parser::Authority,
                                            authority))

BOOST_FUSION_ADAPT_STRUCT(gis::srs::wkt_parser::Projection,
                          (gis::srs::wkt_parser::String,
                           name)(gis::srs::wkt_parser::Authority, authority))

BOOST_FUSION_ADAPT_STRUCT(gis::srs::wkt_parser::Projection_parameter,
                          (gis::srs::wkt_parser::String, name), (double, value),
                          (gis::srs::wkt_parser::Authority, authority))

BOOST_FUSION_ADAPT_STRUCT(
    gis::srs::wkt_parser::Projected_cs,
    (gis::srs::wkt_parser::String,
     name)(gis::srs::wkt_parser::Geographic_cs,
           geographic_cs)(gis::srs::wkt_parser::Projection, projection)(
        gis::srs::wkt_parser::Projection_parameters,
        parameters)(gis::srs::wkt_parser::Unit,
                    linear_unit)(gis::srs::wkt_parser::Twin_axes,
                                 axes)(gis::srs::wkt_parser::Authority,
                                       authority))

namespace gis {
namespace srs {
namespace wkt_parser {

/// @cond DOXYGEN_DOESNT_UNDERSTAND_THIS_LINE
namespace qi = boost::spirit::qi;
/// @endcond

template <typename T>
struct number_policies : qi::real_policies<T> {
  template <typename Iterator, typename Attribute>
  static bool parse_nan(Iterator &, Iterator const &, Attribute &) {
    return false;
  }

  template <typename Iterator, typename Attribute>
  static bool parse_inf(Iterator &, Iterator const &, Attribute &) {
    return false;
  }
};

template <typename Iterator, typename Skipper>
struct Grammar : qi::grammar<Iterator, Coordinate_system(), Skipper> {
  Grammar(char right_delimiter_char) : Grammar::base_type(start) {
    if (right_delimiter_char == ')') {
      left_delimiter = qi::lit('(');
      right_delimiter = qi::lit(')');
    } else {
      left_delimiter = qi::lit('[');
      right_delimiter = qi::lit(']');
    }
    quoted_string = qi::lexeme['"' >> +(qi::char_ - '"') >> '"'];
    authority = qi::no_case[qi::lit("AUTHORITY")] >> qi::attr(true) >>
                left_delimiter >> quoted_string >> qi::lit(',') >>
                quoted_string >> right_delimiter;
    number = qi::real_parser<double, number_policies<double>>();
    unit = qi::no_case[qi::lit("UNIT")] >> left_delimiter >> quoted_string >>
           qi::lit(',') >> number >> -(qi::lit(',') >> authority) >>
           right_delimiter;
    linear_unit = unit;
    angular_unit = unit;
    spheroid = qi::no_case[qi::lit("SPHEROID")] >> left_delimiter >>
               quoted_string >> qi::lit(',') >> number >> qi::lit(',') >>
               number >> -(qi::lit(',') >> authority) >> right_delimiter;
    to_wgs84 = qi::no_case[qi::lit("TOWGS84")] >> qi::attr(true) >>
               left_delimiter >> number >>  // dx
               qi::lit(',') >> number >>    // dy
               qi::lit(',') >> number >>    // dz
               qi::lit(',') >> number >>    // ex
               qi::lit(',') >> number >>    // ey
               qi::lit(',') >> number >>    // ez
               qi::lit(',') >> number >>    // ppm
               right_delimiter;
    datum = qi::no_case[qi::lit("DATUM")] >> left_delimiter >> quoted_string >>
            qi::lit(',') >> spheroid >> -(qi::lit(',') >> to_wgs84) >>
            -(qi::lit(',') >> authority) >> right_delimiter;
    prime_meridian = qi::no_case[qi::lit("PRIMEM")] >> left_delimiter >>
                     quoted_string >> qi::lit(',') >> number >>
                     -(qi::lit(',') >> authority) >> right_delimiter;
    axis_direction.add
        // All values must be lower case to achieve case insensitive
        // matching.
        ("north", Axis_direction::NORTH)("south", Axis_direction::SOUTH)(
            "east", Axis_direction::EAST)("west", Axis_direction::WEST)(
            "other", Axis_direction::OTHER);
    axis = qi::no_case[qi::lit("AXIS")] >> left_delimiter >> quoted_string >>
           qi::lit(',') >> qi::no_case[axis_direction] >> right_delimiter;
    twin_axes = qi::attr(true) >> axis >> qi::lit(',') >> axis;
    geographic_cs = qi::no_case[qi::lit("GEOGCS")] >> left_delimiter >>
                    quoted_string >> qi::lit(',') >> datum >> qi::lit(',') >>
                    prime_meridian >> qi::lit(',') >> angular_unit >>
                    qi::lit(',') >> twin_axes >> -(qi::lit(',') >> authority) >>
                    right_delimiter;
    parameter = qi::no_case[qi::lit("PARAMETER")] >> left_delimiter >>
                quoted_string >> qi::lit(',') >> number >>
                -(qi::lit(',') >> authority) >> right_delimiter;
    parameters = parameter % qi::lit(',');
    projection = qi::no_case[qi::lit("PROJECTION")] >> left_delimiter >>
                 quoted_string >> -(qi::lit(',') >> authority) >>
                 right_delimiter;
    projected_cs = qi::no_case[qi::lit("PROJCS")] >> left_delimiter >>
                   quoted_string >> qi::lit(',') >> geographic_cs >>
                   qi::lit(',') >> projection >>
                   -(qi::lit(',') >> parameters) >> qi::lit(',') >>
                   linear_unit >> -(qi::lit(',') >> twin_axes) >>
                   -(qi::lit(',') >> authority) >> right_delimiter;
    horz_cs = projected_cs | geographic_cs;
    coordinate_system = horz_cs;
    start = coordinate_system;
  }

  qi::rule<Iterator> left_delimiter;
  qi::rule<Iterator> right_delimiter;
  qi::rule<Iterator, String()> quoted_string;
  qi::rule<Iterator, Authority(), Skipper> authority;
  qi::rule<Iterator, double(), Skipper> number;
  qi::rule<Iterator, Unit(), Skipper> unit;
  qi::rule<Iterator, Unit(), Skipper> linear_unit;
  qi::rule<Iterator, Unit(), Skipper> angular_unit;
  qi::rule<Iterator, Spheroid(), Skipper> spheroid;
  qi::rule<Iterator, Towgs84(), Skipper> to_wgs84;
  qi::rule<Iterator, Datum(), Skipper> datum;
  qi::rule<Iterator, Prime_meridian(), Skipper> prime_meridian;
  qi::symbols<char, Axis_direction> axis_direction;
  qi::rule<Iterator, Axis(), Skipper> axis;
  qi::rule<Iterator, Twin_axes(), Skipper> twin_axes;
  qi::rule<Iterator, Geographic_cs(), Skipper> geographic_cs;
  qi::rule<Iterator, Projection_parameter(), Skipper> parameter;
  qi::rule<Iterator, Projection_parameters(), Skipper> parameters;
  qi::rule<Iterator, Projection(), Skipper> projection;
  qi::rule<Iterator, Projected_cs(), Skipper> projected_cs;
  qi::rule<Iterator, Coordinate_system(), Skipper> horz_cs;
  qi::rule<Iterator, Coordinate_system(), Skipper> coordinate_system;
  qi::rule<Iterator, Coordinate_system(), Skipper> start;
};

}  // namespace wkt_parser
}  // namespace srs
}  // namespace gis

bool gis::srs::wkt_parser::parse_wkt(
    srid_t srid, const char *begin, const char *end,
    gis::srs::wkt_parser::Coordinate_system *cs) {
  // gis::srs::parse_wkt() should have filtered these out already
  DBUG_ASSERT(begin != nullptr && begin < (end - 1));

  namespace wp = gis::srs::wkt_parser;

  bool res = false;
  const char *it = begin;

  const char *delimiter = end;
  delimiter--;
  while (delimiter > begin && std::isspace(*delimiter)) delimiter--;

  wp::Grammar<decltype(delimiter), boost::spirit::ascii::space_type> g(
      *delimiter);

  try {
    res = !boost::spirit::qi::phrase_parse(it, end, g,
                                           boost::spirit::ascii::space, *cs);
    if (it != end || res) {
      my_error(ER_SRS_PARSE_ERROR, MYF(0), srid);
      res = true;
    }
  } catch (...) {
    my_error(ER_SRS_PARSE_ERROR, MYF(0), srid);
    res = true;
  }

  return res;
}
