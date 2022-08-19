/*
   Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @file sql/item_create.cc

  Functions to create an item. Used by sql_yacc.yy
*/

#include "sql/item_create.h"

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <algorithm>
#include <cctype>
#include <iterator>
#include <limits>
#include <new>
#include <string>
#include <unordered_map>
#include <utility>

#include "decimal.h"
#include "field_types.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "my_time.h"
#include "mysql/udf_registration_types.h"
#include "mysql_time.h"
#include "mysqld_error.h"
#include "sql/item.h"
#include "sql/item_cmpfunc.h"      // Item_func_any_value
#include "sql/item_func.h"         // Item_func_udf_str
#include "sql/item_geofunc.h"      // Item_func_st_area
#include "sql/item_inetfunc.h"     // Item_func_inet_ntoa
#include "sql/item_json_func.h"    // Item_func_json
#include "sql/item_pfs_func.h"     // Item_pfs_func_thread_id
#include "sql/item_regexp_func.h"  // Item_func_regexp_xxx
#include "sql/item_strfunc.h"      // Item_func_aes_encrypt
#include "sql/item_sum.h"          // Item_sum_udf_str
#include "sql/item_timefunc.h"     // Item_func_add_time
#include "sql/item_xmlfunc.h"      // Item_func_xml_extractvalue
#include "sql/my_decimal.h"
#include "sql/parse_location.h"
#include "sql/parse_tree_helpers.h"  // PT_item_list
#include "sql/parser_yystype.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_exception_handler.h"  // handle_std_exception
#include "sql/sql_lex.h"
#include "sql/sql_time.h"  // str_to_datetime
#include "sql/sql_udf.h"
#include "sql/system_variables.h"
#include "sql_string.h"
#include "tztime.h"  // adjust_time_zone

/**
  @addtogroup GROUP_PARSER
  @{
*/

/**
  @defgroup Instantiators Instantiator functions

  The Instantiator functions are used to call constructors and `operator new`
  on classes that implement SQL functions, basically, even though they don't
  have to be functions. This pattern has to be used because of the
  following reasons:

  - The parser produces PT_item_list objects of all argument lists, while the
    Item_func subclasses use overloaded constructors,
    e.g. Item_xxx_func(Item*), Item_xxx_func(Item*, Item*), etc.

  - We need to map parser tokens to classes and we don't have reflection.

  Because partial template specialization is used, the functions are
  implemented as class templates rather that functions templates.

  Functions objects that can be created simply by calling the constructor of
  their respective Item_func class need only instantiate the first template
  below. Some functions do some special tricks before creating the function
  object, and in that case they need their own Instantiator. See for instance
  Bin_instantiator or Oct_instantiator here below for how to do that.

  Keeping the templates in anonymous namespaces enables the compiler to inline
  more and hence keeps the generated code leaner.

  @{
*/

/**
  We use this to declare that a function takes an infinite number of
  arguments. The cryptic construction below gives us the greatest number that
  the return type of PT_item_list::elements() can take.

  @see Function_factory::create_func()
*/
static const auto MAX_ARGLIST_SIZE =
    std::numeric_limits<decltype(PT_item_list().elements())>::max();

namespace {

/**
  Instantiates a function class with the list of arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.

  @tparam Min_argc The minimum number of arguments. Not used in this
  general case.

  @tparam Max_argc The maximum number of arguments. Not used in this
  general case.
*/

template <typename Function_class, uint Min_argc, uint Max_argc = Min_argc>
class Instantiator {
 public:
  static const uint Min_argcount = Min_argc;
  static const uint Max_argcount = Max_argc;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Function_class(POS(), args);
  }
};

/**
  Instantiates a function class with no arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator<Function_class, 0> {
 public:
  static const uint Min_argcount = 0;
  static const uint Max_argcount = 0;
  Item *instantiate(THD *thd, PT_item_list *) {
    return new (thd->mem_root) Function_class(POS());
  }
};

template <typename Function_class, uint Min_argc, uint Max_argc = Min_argc>
class Instantiator_with_thd {
 public:
  static const uint Min_argcount = Min_argc;
  static const uint Max_argcount = Max_argc;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Function_class(thd, POS(), args);
  }
};

template <typename Function_class, Item_func::Functype Functype, uint Min_argc,
          uint Max_argc = Min_argc>
class Instantiator_with_functype {
 public:
  static const uint Min_argcount = Min_argc;
  static const uint Max_argcount = Max_argc;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Function_class(thd, POS(), args, Functype);
  }
};

template <typename Function_class, Item_func::Functype Function_type>
class Instantiator_with_functype<Function_class, Function_type, 1, 1> {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 1;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Function_class(POS(), (*args)[0], Function_type);
  }
};

template <typename Function_class, Item_func::Functype Function_type>
class Instantiator_with_functype<Function_class, Function_type, 2, 2> {
 public:
  static const uint Min_argcount = 2;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root)
        Function_class(POS(), (*args)[0], (*args)[1], Function_type);
  }
};

template <typename Function_class, uint Min_argc, uint Max_argc = Min_argc>
class List_instantiator {
 public:
  static const uint Min_argcount = Min_argc;
  static const uint Max_argcount = Max_argc;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Function_class(POS(), args);
  }
};

template <typename Function_class, uint Min_argc, uint Max_argc = Min_argc>
class List_instantiator_with_thd {
 public:
  static const uint Min_argcount = Min_argc;
  static const uint Max_argcount = Max_argc;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Function_class(thd, POS(), args);
  }
};

/**
  Instantiates a function class with one argument.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator<Function_class, 1> {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 1;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Function_class(POS(), (*args)[0]);
  }
};

/**
  Instantiates a function class with two arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator<Function_class, 2> {
 public:
  static const uint Min_argcount = 2;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Function_class(POS(), (*args)[0], (*args)[1]);
  }
};

/**
  Instantiates a function class with three arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator<Function_class, 3> {
 public:
  static const uint Min_argcount = 3;
  static const uint Max_argcount = 3;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root)
        Function_class(POS(), (*args)[0], (*args)[1], (*args)[2]);
  }
};

/**
  Instantiates a function class with four arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator<Function_class, 4> {
 public:
  static const uint Min_argcount = 4;
  static const uint Max_argcount = 4;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root)
        Function_class(POS(), (*args)[0], (*args)[1], (*args)[2], (*args)[3]);
  }
};

/**
  Instantiates a function class with five arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator<Function_class, 5> {
 public:
  static const uint Min_argcount = 5;
  static const uint Max_argcount = 5;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Function_class(
        POS(), (*args)[0], (*args)[1], (*args)[2], (*args)[3], (*args)[4]);
  }
};

/**
  Instantiates a function class with zero or one arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator<Function_class, 0, 1> {
 public:
  static const uint Min_argcount = 0;
  static const uint Max_argcount = 1;

  Item *instantiate(THD *thd, PT_item_list *args) {
    uint argcount = args == nullptr ? 0 : args->elements();
    switch (argcount) {
      case 0:
        return new (thd->mem_root) Function_class(POS());
      case 1:
        return new (thd->mem_root) Function_class(POS(), (*args)[0]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

/**
  Instantiates a function class with one or two arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator<Function_class, 1, 2> {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1:
        return new (thd->mem_root) Function_class(POS(), (*args)[0]);
      case 2:
        return new (thd->mem_root)
            Function_class(POS(), (*args)[0], (*args)[1]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

/**
  Instantiates a function class with between one and three arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator<Function_class, 1, 3> {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 3;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1:
        return new (thd->mem_root) Function_class(POS(), (*args)[0]);
      case 2:
        return new (thd->mem_root)
            Function_class(POS(), (*args)[0], (*args)[1]);
      case 3:
        return new (thd->mem_root)
            Function_class(POS(), (*args)[0], (*args)[1], (*args)[2]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

/**
  Instantiates a function class taking between one and three arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator_with_thd<Function_class, 1, 3> {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 3;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1:
        return new (thd->mem_root) Function_class(thd, POS(), (*args)[0]);
      case 2:
        return new (thd->mem_root)
            Function_class(thd, POS(), (*args)[0], (*args)[1]);
      case 3:
        return new (thd->mem_root)
            Function_class(thd, POS(), (*args)[0], (*args)[1], (*args)[2]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

/**
  Instantiates a function class taking a thd and one or two arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator_with_thd<Function_class, 1, 2> {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1:
        return new (thd->mem_root) Function_class(thd, POS(), (*args)[0]);
      case 2:
        return new (thd->mem_root)
            Function_class(thd, POS(), (*args)[0], (*args)[1]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

/**
  Instantiates a function class with two or three arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator<Function_class, 2, 3> {
 public:
  static const uint Min_argcount = 2;
  static const uint Max_argcount = 3;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 2:
        return new (thd->mem_root)
            Function_class(POS(), (*args)[0], (*args)[1]);
      case 3:
        return new (thd->mem_root)
            Function_class(POS(), (*args)[0], (*args)[1], (*args)[2]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

/**
  Instantiates a function class with between two and four arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator<Function_class, 2, 4> {
 public:
  static const uint Min_argcount = 2;
  static const uint Max_argcount = 4;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 2:
        return new (thd->mem_root)
            Function_class(POS(), (*args)[0], (*args)[1]);
      case 3:
        return new (thd->mem_root)
            Function_class(POS(), (*args)[0], (*args)[1], (*args)[2]);
      case 4:
        return new (thd->mem_root) Function_class(POS(), (*args)[0], (*args)[1],
                                                  (*args)[2], (*args)[3]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

/**
  Instantiates a function class with two or three arguments.

  @tparam Function_class The class that implements the function. Does not need
  to inherit Item_func.
*/
template <typename Function_class>
class Instantiator<Function_class, 3, 5> {
 public:
  static const uint Min_argcount = 3;
  static const uint Max_argcount = 5;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 3:
        return new (thd->mem_root)
            Function_class(POS(), (*args)[0], (*args)[1], (*args)[2]);
      case 4:
        return new (thd->mem_root) Function_class(POS(), (*args)[0], (*args)[1],
                                                  (*args)[2], (*args)[3]);
      case 5:
        return new (thd->mem_root) Function_class(
            POS(), (*args)[0], (*args)[1], (*args)[2], (*args)[3], (*args)[4]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

template <Item_func::Functype Functype>
using Spatial_decomp_instantiator =
    Instantiator_with_functype<Item_func_spatial_decomp, Functype, 1>;

using Startpoint_instantiator =
    Spatial_decomp_instantiator<Item_func::SP_STARTPOINT>;
using Endpoint_instantiator =
    Spatial_decomp_instantiator<Item_func::SP_ENDPOINT>;
using Exteriorring_instantiator =
    Spatial_decomp_instantiator<Item_func::SP_EXTERIORRING>;

template <Item_func::Functype Functype>
using Spatial_decomp_n_instantiator =
    Instantiator_with_functype<Item_func_spatial_decomp_n, Functype, 2>;

using Sp_geometryn_instantiator =
    Spatial_decomp_n_instantiator<Item_func::SP_GEOMETRYN>;

using Sp_interiorringn_instantiator =
    Spatial_decomp_n_instantiator<Item_func::SP_INTERIORRINGN>;

using Sp_pointn_instantiator =
    Spatial_decomp_n_instantiator<Item_func::SP_POINTN>;

template <typename Geometry_class, enum Geometry_class::Functype Functype>
class Geometry_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 3;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1:
        return new (thd->mem_root) Geometry_class(POS(), (*args)[0], Functype);
      case 2:
        return new (thd->mem_root)
            Geometry_class(POS(), (*args)[0], (*args)[1], Functype);
      case 3:
        return new (thd->mem_root)
            Geometry_class(POS(), (*args)[0], (*args)[1], (*args)[2], Functype);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

using txt_ft = Item_func_geometry_from_text::Functype;
using I_txt = Item_func_geometry_from_text;
template <typename Geometry_class, enum Geometry_class::Functype Functype>
using G_i = Geometry_instantiator<Geometry_class, Functype>;

using Geomcollfromtext_instantiator = G_i<I_txt, txt_ft::GEOMCOLLFROMTEXT>;
using Geomcollfromtxt_instantiator = G_i<I_txt, txt_ft::GEOMCOLLFROMTXT>;
using Geometrycollectionfromtext_instantiator =
    G_i<I_txt, txt_ft::GEOMETRYCOLLECTIONFROMTEXT>;
using Geometryfromtext_instantiator = G_i<I_txt, txt_ft::GEOMETRYFROMTEXT>;
using Geomfromtext_instantiator = G_i<I_txt, txt_ft::GEOMFROMTEXT>;
using Linefromtext_instantiator = G_i<I_txt, txt_ft::LINEFROMTEXT>;
using Linestringfromtext_instantiator = G_i<I_txt, txt_ft::LINESTRINGFROMTEXT>;
using Mlinefromtext_instantiator = G_i<I_txt, txt_ft::MLINEFROMTEXT>;
using Mpointfromtext_instantiator = G_i<I_txt, txt_ft::MPOINTFROMTEXT>;
using Mpolyfromtext_instantiator = G_i<I_txt, txt_ft::MPOLYFROMTEXT>;
using Multilinestringfromtext_instantiator =
    G_i<I_txt, txt_ft::MULTILINESTRINGFROMTEXT>;
using Multipointfromtext_instantiator = G_i<I_txt, txt_ft::MULTIPOINTFROMTEXT>;
using Multipolygonfromtext_instantiator =
    G_i<I_txt, txt_ft::MULTIPOLYGONFROMTEXT>;
using Pointfromtext_instantiator = G_i<I_txt, txt_ft::POINTFROMTEXT>;
using Polyfromtext_instantiator = G_i<I_txt, txt_ft::POLYFROMTEXT>;
using Polygonfromtext_instantiator = G_i<I_txt, txt_ft::POLYGONFROMTEXT>;

using wkb_ft = Item_func_geometry_from_wkb::Functype;
using I_wkb = Item_func_geometry_from_wkb;

using Geomcollfromwkb_instantiator = G_i<I_wkb, wkb_ft::GEOMCOLLFROMWKB>;
using Geometrycollectionfromwkb_instantiator =
    G_i<I_wkb, wkb_ft::GEOMETRYCOLLECTIONFROMWKB>;
using Geometryfromwkb_instantiator = G_i<I_wkb, wkb_ft::GEOMETRYFROMWKB>;
using Geomfromwkb_instantiator = G_i<I_wkb, wkb_ft::GEOMFROMWKB>;
using Linefromwkb_instantiator = G_i<I_wkb, wkb_ft::LINEFROMWKB>;
using Linestringfromwkb_instantiator = G_i<I_wkb, wkb_ft::LINESTRINGFROMWKB>;
using Mlinefromwkb_instantiator = G_i<I_wkb, wkb_ft::MLINEFROMWKB>;
using Mpointfromwkb_instantiator = G_i<I_wkb, wkb_ft::MPOINTFROMWKB>;
using Mpolyfromwkb_instantiator = G_i<I_wkb, wkb_ft::MPOLYFROMWKB>;
using Multilinestringfromwkb_instantiator =
    G_i<I_wkb, wkb_ft::MULTILINESTRINGFROMWKB>;
using Multipointfromwkb_instantiator = G_i<I_wkb, wkb_ft::MULTIPOINTFROMWKB>;
using Multipolygonfromwkb_instantiator =
    G_i<I_wkb, wkb_ft::MULTIPOLYGONFROMWKB>;
using Pointfromwkb_instantiator = G_i<I_wkb, wkb_ft::POINTFROMWKB>;
using Polyfromwkb_instantiator = G_i<I_wkb, wkb_ft::POLYFROMWKB>;
using Polygonfromwkb_instantiator = G_i<I_wkb, wkb_ft::POLYGONFROMWKB>;

}  // namespace

class Bin_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 1;

  Item *instantiate(THD *thd, PT_item_list *args) {
    POS pos;
    Item *i10 = new (thd->mem_root) Item_int(pos, 10, 2);
    Item *i2 = new (thd->mem_root) Item_int(pos, 2, 1);
    return new (thd->mem_root) Item_func_conv(pos, (*args)[0], i10, i2);
  }
};

class Oct_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 1;

  Item *instantiate(THD *thd, PT_item_list *args) {
    Item *i10 = new (thd->mem_root) Item_int(POS(), 10, 2);
    Item *i8 = new (thd->mem_root) Item_int(POS(), 8, 1);
    return new (thd->mem_root) Item_func_conv(POS(), (*args)[0], i10, i8);
  }
};

class Weekday_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 1;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Item_func_weekday(POS(), (*args)[0], false);
  }
};

class Weekofyear_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 1;

  Item *instantiate(THD *thd, PT_item_list *args) {
    Item *i1 = new (thd->mem_root) Item_int(POS(), NAME_STRING("0"), 3, 1);
    return new (thd->mem_root) Item_func_week(POS(), (*args)[0], i1);
  }
};

class Datediff_instantiator {
 public:
  static const uint Min_argcount = 2;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    Item *i1 = new (thd->mem_root) Item_func_to_days(POS(), (*args)[0]);
    Item *i2 = new (thd->mem_root) Item_func_to_days(POS(), (*args)[1]);

    return new (thd->mem_root) Item_func_minus(POS(), i1, i2);
  }
};

class Subtime_instantiator {
 public:
  static const uint Min_argcount = 2;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root)
        Item_func_add_time(POS(), (*args)[0], (*args)[1], false, true);
  }
};

class Time_format_instantiator {
 public:
  static const uint Min_argcount = 2;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root)
        Item_func_date_format(POS(), (*args)[0], (*args)[1], true);
  }
};

class Dayofweek_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 1;

  Item *instantiate(THD *thd, PT_item_list *args) {
    return new (thd->mem_root) Item_func_weekday(POS(), (*args)[0], true);
  }
};

class From_unixtime_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1:
        return new (thd->mem_root) Item_func_from_unixtime(POS(), (*args)[0]);
      case 2: {
        Item *ut =
            new (thd->mem_root) Item_func_from_unixtime(POS(), (*args)[0]);
        return new (thd->mem_root)
            Item_func_date_format(POS(), ut, (*args)[1], false);
      }
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

class Round_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1: {
        Item *i0 = new (thd->mem_root) Item_int_0(POS());
        return new (thd->mem_root)
            Item_func_round(POS(), (*args)[0], i0, false);
      }
      case 2:
        return new (thd->mem_root)
            Item_func_round(POS(), (*args)[0], (*args)[1], false);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

class Locate_instantiator {
 public:
  static const uint Min_argcount = 2;
  static const uint Max_argcount = 3;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 2:
        /* Yes, parameters in that order : 2, 1 */
        return new (thd->mem_root)
            Item_func_locate(POS(), (*args)[1], (*args)[0]);
      case 3:
        /* Yes, parameters in that order : 2, 1, 3 */
        return new (thd->mem_root)
            Item_func_locate(POS(), (*args)[1], (*args)[0], (*args)[2]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

class Srid_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1:
        return new (thd->mem_root)
            Item_func_st_srid_observer(POS(), (*args)[0]);
      case 2:
        return new (thd->mem_root)
            Item_func_st_srid_mutator(POS(), (*args)[0], (*args)[1]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

class Latitude_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1:
        return new (thd->mem_root)
            Item_func_st_latitude_observer(POS(), (*args)[0]);
      case 2:
        return new (thd->mem_root)
            Item_func_st_latitude_mutator(POS(), (*args)[0], (*args)[1]);
      default:
        /* purecov: begin deadcode */
        DBUG_ASSERT(false);
        return nullptr;
        /* purecov: end */
    }
  }
};

class Longitude_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1:
        return new (thd->mem_root)
            Item_func_st_longitude_observer(POS(), (*args)[0]);
      case 2:
        return new (thd->mem_root)
            Item_func_st_longitude_mutator(POS(), (*args)[0], (*args)[1]);
      default:
        /* purecov: begin deadcode */
        DBUG_ASSERT(false);
        return nullptr;
        /* purecov: end */
    }
  }
};

class X_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1:
        return new (thd->mem_root) Item_func_st_x_observer(POS(), (*args)[0]);
      case 2:
        return new (thd->mem_root)
            Item_func_st_x_mutator(POS(), (*args)[0], (*args)[1]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

class Y_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1:
        return new (thd->mem_root) Item_func_st_y_observer(POS(), (*args)[0]);
      case 2:
        return new (thd->mem_root)
            Item_func_st_y_mutator(POS(), (*args)[0], (*args)[1]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

class Yearweek_instantiator {
 public:
  static const uint Min_argcount = 1;
  static const uint Max_argcount = 2;

  Item *instantiate(THD *thd, PT_item_list *args) {
    switch (args->elements()) {
      case 1: {
        Item *i0 = new (thd->mem_root) Item_int_0(POS());
        return new (thd->mem_root) Item_func_yearweek(POS(), (*args)[0], i0);
      }
      case 2:
        return new (thd->mem_root)
            Item_func_yearweek(POS(), (*args)[0], (*args)[1]);
      default:
        DBUG_ASSERT(false);
        return nullptr;
    }
  }
};

class Make_set_instantiator {
 public:
  static const uint Min_argcount = 2;
  static const uint Max_argcount = MAX_ARGLIST_SIZE;

  Item *instantiate(THD *thd, PT_item_list *args) {
    Item *param_1 = args->pop_front();
    return new (thd->mem_root) Item_func_make_set(POS(), param_1, args);
  }
};

/// @} (end of group Instantiators)

uint arglist_length(const PT_item_list *args) {
  if (args == nullptr) return 0;
  return args->elements();
}

bool check_argcount_bounds(THD *, LEX_STRING function_name,
                           PT_item_list *item_list, uint min_argcount,
                           uint max_argcount) {
  uint argcount = arglist_length(item_list);
  if (argcount < min_argcount || argcount > max_argcount) {
    my_error(ER_WRONG_PARAMCOUNT_TO_NATIVE_FCT, MYF(0), function_name.str);
    return true;
  }
  return false;
}

namespace {

/**
  Factory for creating function objects. Performs validation check that the
  number of arguments is correct, then calls upon the instantiator function to
  instantiate the function object.

  @tparam Instantiator_fn A class that is expected to contain the following:

  - Min_argcount: The minimal number of arguments required to call the
  function. If the parameter count is less, an SQL error is raised and nullptr
  is returned.

  - Max_argcount: The maximum number of arguments required to call the
  function. If the parameter count is greater, an SQL error is raised and
  nullptr is returned.

  - Item *instantiate(THD *, PT_item_list *): Should construct an Item.
*/
template <typename Instantiator_fn>
class Function_factory : public Create_func {
 public:
  static Function_factory<Instantiator_fn> s_singleton;

  Item *create_func(THD *thd, LEX_STRING function_name,
                    PT_item_list *item_list) override {
    if (check_argcount_bounds(thd, function_name, item_list,
                              m_instantiator.Min_argcount,
                              m_instantiator.Max_argcount))
      return nullptr;
    return m_instantiator.instantiate(thd, item_list);
  }

 private:
  Function_factory() {}
  Instantiator_fn m_instantiator;
};

template <typename Instantiator_fn>
Function_factory<Instantiator_fn>
    Function_factory<Instantiator_fn>::s_singleton;

template <typename Instantiator_fn>
class Odd_argcount_function_factory : public Create_func {
 public:
  static Odd_argcount_function_factory<Instantiator_fn> s_singleton;

  Item *create_func(THD *thd, LEX_STRING function_name,
                    PT_item_list *item_list) override {
    if (check_argcount_bounds(thd, function_name, item_list,
                              m_instantiator.Min_argcount,
                              m_instantiator.Max_argcount))
      return nullptr;
    if (arglist_length(item_list) % 2 == 0) {
      my_error(ER_WRONG_PARAMCOUNT_TO_NATIVE_FCT, MYF(0), function_name.str);
      return nullptr;
    }
    return m_instantiator.instantiate(thd, item_list);
  }

 private:
  Odd_argcount_function_factory() {}
  Instantiator_fn m_instantiator;
};

template <typename Instantiator_fn>
Odd_argcount_function_factory<Instantiator_fn>
    Odd_argcount_function_factory<Instantiator_fn>::s_singleton;

template <typename Instantiator_fn>
class Even_argcount_function_factory : public Create_func {
 public:
  static Even_argcount_function_factory<Instantiator_fn> s_singleton;

  Item *create_func(THD *thd, LEX_STRING function_name,
                    PT_item_list *item_list) override {
    if (check_argcount_bounds(thd, function_name, item_list,
                              m_instantiator.Min_argcount,
                              m_instantiator.Max_argcount))
      return nullptr;
    if (arglist_length(item_list) % 2 != 0) {
      my_error(ER_WRONG_PARAMCOUNT_TO_NATIVE_FCT, MYF(0), function_name.str);
      return nullptr;
    }
    return m_instantiator.instantiate(thd, item_list);
  }

 private:
  Even_argcount_function_factory() {}
  Instantiator_fn m_instantiator;
};

template <typename Instantiator_fn>
Even_argcount_function_factory<Instantiator_fn>
    Even_argcount_function_factory<Instantiator_fn>::s_singleton;

/**
  Factory for internal functions that should be invoked from the system views
  only.

  @tparam Instantiator_fn See Function_factory.
*/
template <typename Instantiator_fn>
class Internal_function_factory : public Create_func {
 public:
  static Internal_function_factory<Instantiator_fn> s_singleton;

  Item *create_func(THD *thd, LEX_STRING function_name,
                    PT_item_list *item_list) override {
    if (!thd->parsing_system_view && !thd->is_dd_system_thread() &&
        DBUG_EVALUATE_IF("skip_dd_table_access_check", false, true)) {
      my_error(ER_NO_ACCESS_TO_NATIVE_FCT, MYF(0), function_name.str);
      return nullptr;
    }

    if (check_argcount_bounds(thd, function_name, item_list,
                              m_instantiator.Min_argcount,
                              m_instantiator.Max_argcount))
      return nullptr;
    return m_instantiator.instantiate(thd, item_list);
  }

 private:
  Internal_function_factory() {}
  Instantiator_fn m_instantiator;
};

template <typename Instantiator_fn>
Internal_function_factory<Instantiator_fn>
    Internal_function_factory<Instantiator_fn>::s_singleton;

}  // namespace

/**
  Function builder for stored functions.
*/
class Create_sp_func : public Create_qfunc {
 public:
  virtual Item *create(THD *thd, LEX_STRING db, LEX_STRING name,
                       bool use_explicit_name, PT_item_list *item_list);

  static Create_sp_func s_singleton;

 protected:
  /** Constructor. */
  Create_sp_func() {}
  /** Destructor. */
  virtual ~Create_sp_func() {}
};

Item *Create_qfunc::create_func(THD *thd, LEX_STRING name,
                                PT_item_list *item_list) {
  return create(thd, NULL_STR, name, false, item_list);
}

Create_udf_func Create_udf_func::s_singleton;

Item *Create_udf_func::create_func(THD *thd, LEX_STRING name,
                                   PT_item_list *item_list) {
  udf_func *udf = find_udf(name.str, name.length);
  DBUG_ASSERT(udf);
  return create(thd, udf, item_list);
}

Item *Create_udf_func::create(THD *thd, udf_func *udf,
                              PT_item_list *item_list) {
  DBUG_TRACE;

  DBUG_ASSERT((udf->type == UDFTYPE_FUNCTION) ||
              (udf->type == UDFTYPE_AGGREGATE));

  Item *func = nullptr;
  POS pos;

  switch (udf->returns) {
    case STRING_RESULT:
      if (udf->type == UDFTYPE_FUNCTION)
        func = new (thd->mem_root) Item_func_udf_str(pos, udf, item_list);
      else
        func = new (thd->mem_root) Item_sum_udf_str(pos, udf, item_list);
      break;
    case REAL_RESULT:
      if (udf->type == UDFTYPE_FUNCTION)
        func = new (thd->mem_root) Item_func_udf_float(pos, udf, item_list);
      else
        func = new (thd->mem_root) Item_sum_udf_float(pos, udf, item_list);
      break;
    case INT_RESULT:
      if (udf->type == UDFTYPE_FUNCTION)
        func = new (thd->mem_root) Item_func_udf_int(pos, udf, item_list);
      else
        func = new (thd->mem_root) Item_sum_udf_int(pos, udf, item_list);
      break;
    case DECIMAL_RESULT:
      if (udf->type == UDFTYPE_FUNCTION)
        func = new (thd->mem_root) Item_func_udf_decimal(pos, udf, item_list);
      else
        func = new (thd->mem_root) Item_sum_udf_decimal(pos, udf, item_list);
      break;
    default:
      my_error(ER_NOT_SUPPORTED_YET, MYF(0), "UDF return type");
  }
  return func;
}

Create_sp_func Create_sp_func::s_singleton;

Item *Create_sp_func::create(THD *thd, LEX_STRING db, LEX_STRING name,
                             bool use_explicit_name, PT_item_list *item_list) {
  return new (thd->mem_root)
      Item_func_sp(POS(), db, name, use_explicit_name, item_list);
}

/**
  Shorthand macro to reference the singleton instance. This also instantiates
  the Function_factory and Instantiator templates.

  @param F The Item_func that the factory should make.
  @param N Number of arguments that the function accepts.
*/
#define SQL_FN(F, N) &Function_factory<Instantiator<F, N>>::s_singleton

/**
  Shorthand macro to reference the singleton instance when there is a
  specialized intantiatior.

  @param INSTANTIATOR The instantiator class.
*/
#define SQL_FACTORY(INSTANTIATOR) &Function_factory<INSTANTIATOR>::s_singleton

/**
  Use this macro if you want to instantiate the Item_func object like
  `Item_func_xxx::Item_func_xxx(pos, args[0], ..., args[MAX])`

  This also instantiates the Function_factory and Instantiator templates.

  @param F The Item_func that the factory should make.
  @param MIN Number of arguments that the function accepts.
  @param MAX Number of arguments that the function accepts.
*/
#define SQL_FN_V(F, MIN, MAX) \
  &Function_factory<Instantiator<F, MIN, MAX>>::s_singleton

/**
  Use this macro if you want to instantiate the Item_func object like
  `Item_func_xxx::Item_func_xxx(thd, pos, args[0], ..., args[MAX])`

  This also instantiates the Function_factory and Instantiator templates.

  @param F The Item_func that the factory should make.
  @param MIN Number of arguments that the function accepts.
  @param MAX Number of arguments that the function accepts.
*/
#define SQL_FN_V_THD(F, MIN, MAX) \
  &Function_factory<Instantiator_with_thd<F, MIN, MAX>>::s_singleton

/**
  Use this macro if you want to instantiate the Item_func object like
  `Item_func_xxx::Item_func_xxx(pos, item_list)`

  This also instantiates the Function_factory and Instantiator templates.

  @param F The Item_func that the factory should make.
  @param MIN Number of arguments that the function accepts.
  @param MAX Number of arguments that the function accepts.
*/
#define SQL_FN_V_LIST(F, MIN, MAX) \
  &Function_factory<List_instantiator<F, MIN, MAX>>::s_singleton

/**
  Use this macro if you want to instantiate the Item_func object like
  `Item_func_xxx::Item_func_xxx(pos, item_list)`

  This also instantiates the Function_factory and Instantiator templates.

  @param F The Item_func that the factory should make.
  @param N Number of arguments that the function accepts.
*/
#define SQL_FN_LIST(F, N) \
  &Function_factory<List_instantiator<F, N>>::s_singleton

/**
  Use this macro if you want to instantiate the Item_func object like
  `Item_func_xxx::Item_func_xxx(thd, pos, item_list)`

  This also instantiates the Function_factory and Instantiator templates.

  @param F The Item_func that the factory should make.
  @param MIN Number of arguments that the function accepts.
  @param MAX Number of arguments that the function accepts.
*/
#define SQL_FN_V_LIST_THD(F, MIN, MAX) \
  &Function_factory<List_instantiator_with_thd<F, MIN, MAX>>::s_singleton

/**
  Just like SQL_FN_V_THD, but enforces a check that the argument count is odd.
*/
#define SQL_FN_ODD(F, MIN, MAX)   \
  &Odd_argcount_function_factory< \
      List_instantiator_with_thd<F, MIN, MAX>>::s_singleton

/**
  Just like SQL_FN_V_THD, but enforces a check that the argument count is even.
*/
#define SQL_FN_EVEN(F, MIN, MAX)   \
  &Even_argcount_function_factory< \
      List_instantiator_with_thd<F, MIN, MAX>>::s_singleton

/**
  Like SQL_FN, but for functions that may only be referenced from system views.

  @param F The Item_func that the factory should make.
  @param N Number of arguments that the function accepts.
*/
#define SQL_FN_INTERNAL(F, N) \
  &Internal_function_factory<Instantiator<F, N>>::s_singleton

/**
  Just like SQL_FN_INTERNAL, but enforces a check that the argument count
  is even.

  @param F The Item_func that the factory should make.
  @param MIN Number of arguments that the function accepts.
  @param MAX Number of arguments that the function accepts.
*/
#define SQL_FN_INTERNAL_V(F, MIN, MAX) \
  &Internal_function_factory<Instantiator<F, MIN, MAX>>::s_singleton

/**
  Like SQL_FN_LIST, but for functions that may only be referenced from system
  views.

  @param F The Item_func that the factory should make.
  @param N Number of arguments that the function accepts.
*/
#define SQL_FN_LIST_INTERNAL(F, N) \
  &Internal_function_factory<List_instantiator<F, N>>::s_singleton

/**
  Like SQL_FN_LIST, but enforces a check that the argument count
  is within the range specified.

  @param F The Item_func that the factory should make.
  @param MIN Number of arguments that the function accepts.
  @param MAX Number of arguments that the function accepts.
*/
#define SQL_FN_LIST_INTERNAL_V(F, MIN, MAX) \
  &Internal_function_factory<List_instantiator<F, MIN, MAX>>::s_singleton

/**
  MySQL native functions.
  MAINTAINER:
  - Keep sorted for human lookup. At runtime, a hash table is used.
  - do **NOT** conditionally (\#ifdef, \#ifndef) define a function *NAME*:
    doing so will cause user code that works against a `--without-XYZ` binary
    to fail with name collisions against a `--with-XYZ` binary.
  - keep 1 line per entry, it makes `grep | sort` easier
  - Use uppercase (tokens are converted to uppercase before lookup.)

  This can't be constexpr because
  - Sun Studio does not allow the Create_func pointer to be constexpr.
*/
static const std::pair<const char *, Create_func *> func_array[] = {
    {"ABS", SQL_FN(Item_func_abs, 1)},
    {"ACOS", SQL_FN(Item_func_acos, 1)},
    {"ADDTIME", SQL_FN(Item_func_add_time, 2)},
    {"AES_DECRYPT", SQL_FN_V(Item_func_aes_decrypt, 2, 3)},
    {"AES_ENCRYPT", SQL_FN_V(Item_func_aes_encrypt, 2, 3)},
    {"ANY_VALUE", SQL_FN(Item_func_any_value, 1)},
    {"ASIN", SQL_FN(Item_func_asin, 1)},
    {"ATAN", SQL_FN_V(Item_func_atan, 1, 2)},
    {"ATAN2", SQL_FN_V(Item_func_atan, 1, 2)},
    {"BENCHMARK", SQL_FN(Item_func_benchmark, 2)},
    {"BIN", SQL_FACTORY(Bin_instantiator)},
    {"BIN_TO_UUID", SQL_FN_V(Item_func_bin_to_uuid, 1, 2)},
    {"BIT_COUNT", SQL_FN(Item_func_bit_count, 1)},
    {"BIT_LENGTH", SQL_FN(Item_func_bit_length, 1)},
    {"CEIL", SQL_FN(Item_func_ceiling, 1)},
    {"CEILING", SQL_FN(Item_func_ceiling, 1)},
    {"CHARACTER_LENGTH", SQL_FN(Item_func_char_length, 1)},
    {"CHAR_LENGTH", SQL_FN(Item_func_char_length, 1)},
    {"COERCIBILITY", SQL_FN(Item_func_coercibility, 1)},
    {"COMPRESS", SQL_FN(Item_func_compress, 1)},
    {"CONCAT", SQL_FN_V(Item_func_concat, 1, MAX_ARGLIST_SIZE)},
    {"CONCAT_WS", SQL_FN_V(Item_func_concat_ws, 2, MAX_ARGLIST_SIZE)},
    {"CONNECTION_ID", SQL_FN(Item_func_connection_id, 0)},
    {"CONV", SQL_FN(Item_func_conv, 3)},
    {"CONVERT_TZ", SQL_FN(Item_func_convert_tz, 3)},
    {"COS", SQL_FN(Item_func_cos, 1)},
    {"COT", SQL_FN(Item_func_cot, 1)},
    {"CRC32", SQL_FN(Item_func_crc32, 1)},
    {"CURRENT_ROLE", SQL_FN(Item_func_current_role, 0)},
    {"DATEDIFF", SQL_FACTORY(Datediff_instantiator)},
    {"DATE_FORMAT", SQL_FN(Item_func_date_format, 2)},
    {"DAYNAME", SQL_FN(Item_func_dayname, 1)},
    {"DAYOFMONTH", SQL_FN(Item_func_dayofmonth, 1)},
    {"DAYOFWEEK", SQL_FACTORY(Dayofweek_instantiator)},
    {"DAYOFYEAR", SQL_FN(Item_func_dayofyear, 1)},
    {"DEGREES", SQL_FN(Item_func_degrees, 1)},
    {"ELT", SQL_FN_V(Item_func_elt, 2, MAX_ARGLIST_SIZE)},
    {"EXP", SQL_FN(Item_func_exp, 1)},
    {"EXPORT_SET", SQL_FN_V(Item_func_export_set, 3, 5)},
    {"EXTRACTVALUE", SQL_FN(Item_func_xml_extractvalue, 2)},
    {"FIELD", SQL_FN_V(Item_func_field, 2, MAX_ARGLIST_SIZE)},
    {"FIND_IN_SET", SQL_FN(Item_func_find_in_set, 2)},
    {"FLOOR", SQL_FN(Item_func_floor, 1)},
    {"FORMAT_BYTES", SQL_FN(Item_func_pfs_format_bytes, 1)},
    {"FORMAT_PICO_TIME", SQL_FN(Item_func_pfs_format_pico_time, 1)},
    {"FOUND_ROWS", SQL_FN(Item_func_found_rows, 0)},
    {"FROM_BASE64", SQL_FN(Item_func_from_base64, 1)},
    {"FROM_DAYS", SQL_FN(Item_func_from_days, 1)},
    {"FROM_UNIXTIME", SQL_FACTORY(From_unixtime_instantiator)},
    {"GET_LOCK", SQL_FN(Item_func_get_lock, 2)},
    {"GET_INDEX_SIZE_BY_PREFIX",
     SQL_FN_LIST(Item_func_get_index_size_by_prefix, 4)},
    {"GREATEST", SQL_FN_V(Item_func_max, 2, MAX_ARGLIST_SIZE)},
    {"GTID_SUBTRACT", SQL_FN(Item_func_gtid_subtract, 2)},
    {"GTID_SUBSET", SQL_FN(Item_func_gtid_subset, 2)},
    {"HEX", SQL_FN(Item_func_hex, 1)},
    {"IFNULL", SQL_FN(Item_func_ifnull, 2)},
    {"INET_ATON", SQL_FN(Item_func_inet_aton, 1)},
    {"INET_NTOA", SQL_FN(Item_func_inet_ntoa, 1)},
    {"INET6_ATON", SQL_FN(Item_func_inet6_aton, 1)},
    {"INET6_NTOA", SQL_FN(Item_func_inet6_ntoa, 1)},
    {"IS_IPV4", SQL_FN(Item_func_is_ipv4, 1)},
    {"IS_IPV6", SQL_FN(Item_func_is_ipv6, 1)},
    {"IS_IPV4_COMPAT", SQL_FN(Item_func_is_ipv4_compat, 1)},
    {"IS_IPV4_MAPPED", SQL_FN(Item_func_is_ipv4_mapped, 1)},
    {"IS_UUID", SQL_FN(Item_func_is_uuid, 1)},
    {"INSTR", SQL_FN(Item_func_instr, 2)},
    {"ISNULL", SQL_FN(Item_func_isnull, 1)},
    {"JSON_VALID", SQL_FN(Item_func_json_valid, 1)},
    {"JSON_CONTAINS", SQL_FN_V_LIST_THD(Item_func_json_contains, 2, 3)},
    {"JSON_CONTAINS_PATH",
     SQL_FN_V_THD(Item_func_json_contains_path, 3, MAX_ARGLIST_SIZE)},
    {"JSON_CONTAINS_KEY",
     SQL_FN_V_THD(Item_func_json_contains_key, 2, MAX_ARGLIST_SIZE)},
    {"JSON_LENGTH", SQL_FN_V_THD(Item_func_json_length, 1, 2)},
    {"JSON_ARRAY_LENGTH", SQL_FN(Item_func_json_array_length, 1)},
    {"JSON_DEPTH", SQL_FN(Item_func_json_depth, 1)},
    {"JSON_PRETTY", SQL_FN(Item_func_json_pretty, 1)},
    {"JSON_TYPE", SQL_FN(Item_func_json_type, 1)},
    {"JSON_KEYS", SQL_FN_V_THD(Item_func_json_keys, 1, 2)},
    {"JSON_EXTRACT", SQL_FN_V_THD(Item_func_json_extract, 2, MAX_ARGLIST_SIZE)},
    {"JSON_EXTRACT_VALUE",
     SQL_FN_V_THD(Item_func_json_extract_value, 2, MAX_ARGLIST_SIZE)},
    {"JSON_ARRAY_APPEND",
     SQL_FN_ODD(Item_func_json_array_append, 3, MAX_ARGLIST_SIZE)},
    {"JSON_INSERT", SQL_FN_ODD(Item_func_json_insert, 3, MAX_ARGLIST_SIZE)},
    {"JSON_ARRAY_INSERT",
     SQL_FN_ODD(Item_func_json_array_insert, 3, MAX_ARGLIST_SIZE)},
    {"JSON_OBJECT",
     SQL_FN_EVEN(Item_func_json_row_object, 0, MAX_ARGLIST_SIZE)},
    {"JSON_OVERLAPS", SQL_FN(Item_func_json_overlaps, 2)},
    {"JSON_SEARCH", SQL_FN_V_THD(Item_func_json_search, 3, MAX_ARGLIST_SIZE)},
    {"JSON_SET", SQL_FN_ODD(Item_func_json_set, 3, MAX_ARGLIST_SIZE)},
    {"JSON_REPLACE", SQL_FN_ODD(Item_func_json_replace, 3, MAX_ARGLIST_SIZE)},
    {"JSON_ARRAY",
     SQL_FN_V_LIST_THD(Item_func_json_array, 0, MAX_ARGLIST_SIZE)},
    {"JSON_REMOVE",
     SQL_FN_V_LIST_THD(Item_func_json_remove, 2, MAX_ARGLIST_SIZE)},
    {"JSON_MERGE",
     SQL_FN_V_LIST_THD(Item_func_json_merge, 2, MAX_ARGLIST_SIZE)},
    {"JSON_MERGE_PATCH",
     SQL_FN_V_LIST_THD(Item_func_json_merge_patch, 2, MAX_ARGLIST_SIZE)},
    {"JSON_MERGE_PRESERVE",
     SQL_FN_V_LIST_THD(Item_func_json_merge_preserve, 2, MAX_ARGLIST_SIZE)},
    {"JSON_QUOTE", SQL_FN_LIST(Item_func_json_quote, 1)},
    {"JSON_SCHEMA_VALID", SQL_FN(Item_func_json_schema_valid, 2)},
    {"JSON_SCHEMA_VALIDATION_REPORT",
     SQL_FN_V_THD(Item_func_json_schema_validation_report, 2, 2)},
    {"JSON_STORAGE_FREE", SQL_FN(Item_func_json_storage_free, 1)},
    {"JSON_STORAGE_SIZE", SQL_FN(Item_func_json_storage_size, 1)},
    {"JSON_UNQUOTE", SQL_FN_LIST(Item_func_json_unquote, 1)},
    {"IS_FREE_LOCK", SQL_FN(Item_func_is_free_lock, 1)},
    {"IS_USED_LOCK", SQL_FN(Item_func_is_used_lock, 1)},
    {"LAST_DAY", SQL_FN(Item_func_last_day, 1)},
    {"LAST_INSERT_ID", SQL_FN_V(Item_func_last_insert_id, 0, 1)},
    {"LCASE", SQL_FN(Item_func_lower, 1)},
    {"LEAST", SQL_FN_V_LIST(Item_func_min, 2, MAX_ARGLIST_SIZE)},
    {"LENGTH", SQL_FN(Item_func_length, 1)},
#ifndef DBUG_OFF
    {"LIKE_RANGE_MIN", SQL_FN(Item_func_like_range_min, 2)},
    {"LIKE_RANGE_MAX", SQL_FN(Item_func_like_range_max, 2)},
#endif
    {"LN", SQL_FN(Item_func_ln, 1)},
    {"LOAD_FILE", SQL_FN(Item_load_file, 1)},
    {"LOCATE", SQL_FACTORY(Locate_instantiator)},
    {"LOG", SQL_FN_V(Item_func_log, 1, 2)},
    {"LOG10", SQL_FN(Item_func_log10, 1)},
    {"LOG2", SQL_FN(Item_func_log2, 1)},
    {"LOWER", SQL_FN(Item_func_lower, 1)},
    {"LPAD", SQL_FN(Item_func_lpad, 3)},
    {"LTRIM", SQL_FN(Item_func_ltrim, 1)},
    {"MAKEDATE", SQL_FN(Item_func_makedate, 2)},
    {"MAKETIME", SQL_FN(Item_func_maketime, 3)},
    {"MAKE_SET", SQL_FACTORY(Make_set_instantiator)},
    {"MASTER_POS_WAIT", SQL_FN_V(Item_master_pos_wait, 2, 4)},
    {"MBRCONTAINS", SQL_FN(Item_func_mbrcontains, 2)},
    {"MBRCOVEREDBY", SQL_FN(Item_func_mbrcoveredby, 2)},
    {"MBRCOVERS", SQL_FN(Item_func_mbrcovers, 2)},
    {"MBRDISJOINT", SQL_FN(Item_func_mbrdisjoint, 2)},
    {"MBREQUALS", SQL_FN(Item_func_mbrequals, 2)},
    {"MBRINTERSECTS", SQL_FN(Item_func_mbrintersects, 2)},
    {"MBROVERLAPS", SQL_FN(Item_func_mbroverlaps, 2)},
    {"MBRTOUCHES", SQL_FN(Item_func_mbrtouches, 2)},
    {"MBRWITHIN", SQL_FN(Item_func_mbrwithin, 2)},
    {"MD5", SQL_FN(Item_func_md5, 1)},
    {"MONTHNAME", SQL_FN(Item_func_monthname, 1)},
    {"NAME_CONST", SQL_FN(Item_name_const, 2)},
    {"NULLIF", SQL_FN(Item_func_nullif, 2)},
    {"OCT", SQL_FACTORY(Oct_instantiator)},
    {"OCTET_LENGTH", SQL_FN(Item_func_length, 1)},
    {"ORD", SQL_FN(Item_func_ord, 1)},
    {"PERIOD_ADD", SQL_FN(Item_func_period_add, 2)},
    {"PERIOD_DIFF", SQL_FN(Item_func_period_diff, 2)},
    {"PI", SQL_FN(Item_func_pi, 0)},
    {"POW", SQL_FN(Item_func_pow, 2)},
    {"POWER", SQL_FN(Item_func_pow, 2)},
    {"PS_CURRENT_THREAD_ID", SQL_FN(Item_func_pfs_current_thread_id, 0)},
    {"PS_THREAD_ID", SQL_FN(Item_func_pfs_thread_id, 1)},
    {"QUOTE", SQL_FN(Item_func_quote, 1)},
    {"RADIANS", SQL_FN(Item_func_radians, 1)},
    {"RAND", SQL_FN_V(Item_func_rand, 0, 1)},
    {"RANDOM_BYTES", SQL_FN(Item_func_random_bytes, 1)},
    {"REGEXP_INSTR", SQL_FN_V_LIST(Item_func_regexp_instr, 2, 6)},
    {"REGEXP_LIKE", SQL_FN_V_LIST(Item_func_regexp_like, 2, 3)},
    {"REGEXP_REPLACE", SQL_FN_V_LIST(Item_func_regexp_replace, 3, 6)},
    {"REGEXP_SUBSTR", SQL_FN_V_LIST(Item_func_regexp_substr, 2, 5)},
    {"RELEASE_ALL_LOCKS", SQL_FN(Item_func_release_all_locks, 0)},
    {"RELEASE_LOCK", SQL_FN(Item_func_release_lock, 1)},
    {"REVERSE", SQL_FN(Item_func_reverse, 1)},
    {"ROLES_GRAPHML", SQL_FN(Item_func_roles_graphml, 0)},
    {"ROUND", SQL_FACTORY(Round_instantiator)},
    {"RPAD", SQL_FN(Item_func_rpad, 3)},
    {"RTRIM", SQL_FN(Item_func_rtrim, 1)},
    {"SEC_TO_TIME", SQL_FN(Item_func_sec_to_time, 1)},
    {"SHA", SQL_FN(Item_func_sha, 1)},
    {"SHA1", SQL_FN(Item_func_sha, 1)},
    {"SHA2", SQL_FN(Item_func_sha2, 2)},
    {"SIGN", SQL_FN(Item_func_sign, 1)},
    {"SIN", SQL_FN(Item_func_sin, 1)},
    {"SLEEP", SQL_FN(Item_func_sleep, 1)},
    {"SOUNDEX", SQL_FN(Item_func_soundex, 1)},
    {"SPACE", SQL_FN(Item_func_space, 1)},
    {"STATEMENT_DIGEST", SQL_FN(Item_func_statement_digest, 1)},
    {"STATEMENT_DIGEST_TEXT", SQL_FN(Item_func_statement_digest_text, 1)},
    {"WAIT_FOR_EXECUTED_GTID_SET",
     SQL_FN_V(Item_wait_for_executed_gtid_set, 1, 2)},
    {"WAIT_UNTIL_SQL_THREAD_AFTER_GTIDS",
     SQL_FN_V(Item_master_gtid_set_wait, 1, 3)},
    {"SQRT", SQL_FN(Item_func_sqrt, 1)},
    {"STRCMP", SQL_FN(Item_func_strcmp, 2)},
    {"STR_TO_DATE", SQL_FN(Item_func_str_to_date, 2)},
    {"ST_AREA", SQL_FN(Item_func_st_area, 1)},
    {"ST_ASBINARY", SQL_FN_V(Item_func_as_wkb, 1, 2)},
    {"ST_ASGEOJSON", SQL_FN_V_THD(Item_func_as_geojson, 1, 3)},
    {"ST_ASTEXT", SQL_FN_V(Item_func_as_wkt, 1, 2)},
    {"ST_ASWKB", SQL_FN_V(Item_func_as_wkb, 1, 2)},
    {"ST_ASWKT", SQL_FN_V(Item_func_as_wkt, 1, 2)},
    {"ST_BUFFER", SQL_FN_V_LIST(Item_func_buffer, 2, 5)},
    {"ST_BUFFER_STRATEGY", SQL_FN_V_LIST(Item_func_buffer_strategy, 1, 2)},
    {"ST_CENTROID", SQL_FN(Item_func_centroid, 1)},
    {"ST_CONTAINS", SQL_FN(Item_func_st_contains, 2)},
    {"ST_CONVEXHULL", SQL_FN(Item_func_convex_hull, 1)},
    {"ST_CROSSES", SQL_FN(Item_func_st_crosses, 2)},
    {"ST_DIFFERENCE", SQL_FN(Item_func_st_difference, 2)},
    {"ST_DIMENSION", SQL_FN(Item_func_dimension, 1)},
    {"ST_DISJOINT", SQL_FN(Item_func_st_disjoint, 2)},
    {"ST_DISTANCE", SQL_FN_V_LIST(Item_func_distance, 2, 3)},
    {"ST_DISTANCE_SPHERE", SQL_FN_V_LIST(Item_func_st_distance_sphere, 2, 3)},
    {"ST_ENDPOINT", SQL_FACTORY(Endpoint_instantiator)},
    {"ST_ENVELOPE", SQL_FN(Item_func_envelope, 1)},
    {"ST_EQUALS", SQL_FN(Item_func_st_equals, 2)},
    {"ST_EXTERIORRING", SQL_FACTORY(Exteriorring_instantiator)},
    {"ST_GEOHASH", SQL_FN_V(Item_func_geohash, 2, 3)},
    {"ST_GEOMCOLLFROMTEXT", SQL_FACTORY(Geomcollfromtext_instantiator)},
    {"ST_GEOMCOLLFROMTXT", SQL_FACTORY(Geomcollfromtxt_instantiator)},
    {"ST_GEOMCOLLFROMWKB", SQL_FACTORY(Geomcollfromwkb_instantiator)},
    {"ST_GEOMETRYCOLLECTIONFROMTEXT",
     SQL_FACTORY(Geometrycollectionfromtext_instantiator)},
    {"ST_GEOMETRYCOLLECTIONFROMWKB",
     SQL_FACTORY(Geometrycollectionfromwkb_instantiator)},
    {"ST_GEOMETRYFROMTEXT", SQL_FACTORY(Geometryfromtext_instantiator)},
    {"ST_GEOMETRYFROMWKB", SQL_FACTORY(Geometryfromwkb_instantiator)},
    {"ST_GEOMETRYN", SQL_FACTORY(Sp_geometryn_instantiator)},
    {"ST_GEOMETRYTYPE", SQL_FN(Item_func_geometry_type, 1)},
    {"ST_GEOMFROMGEOJSON", SQL_FN_V(Item_func_geomfromgeojson, 1, 3)},
    {"ST_GEOMFROMTEXT", SQL_FACTORY(Geomfromtext_instantiator)},
    {"ST_GEOMFROMWKB", SQL_FACTORY(Geomfromwkb_instantiator)},
    {"ST_INTERIORRINGN", SQL_FACTORY(Sp_interiorringn_instantiator)},
    {"ST_INTERSECTS", SQL_FN(Item_func_st_intersects, 2)},
    {"ST_INTERSECTION", SQL_FN(Item_func_st_intersection, 2)},
    {"ST_ISCLOSED", SQL_FN(Item_func_isclosed, 1)},
    {"ST_ISEMPTY", SQL_FN(Item_func_isempty, 1)},
    {"ST_ISSIMPLE", SQL_FN(Item_func_st_issimple, 1)},
    {"ST_ISVALID", SQL_FN(Item_func_isvalid, 1)},
    {"ST_LATFROMGEOHASH", SQL_FN(Item_func_latfromgeohash, 1)},
    {"ST_LATITUDE", SQL_FACTORY(Latitude_instantiator)},
    {"ST_LENGTH", SQL_FN_V_LIST(Item_func_st_length, 1, 2)},
    {"ST_LINEFROMTEXT", SQL_FACTORY(Linefromtext_instantiator)},
    {"ST_LINEFROMWKB", SQL_FACTORY(Linefromwkb_instantiator)},
    {"ST_LINESTRINGFROMTEXT", SQL_FACTORY(Linestringfromtext_instantiator)},
    {"ST_LINESTRINGFROMWKB", SQL_FACTORY(Linestringfromwkb_instantiator)},
    {"ST_LONGFROMGEOHASH", SQL_FN(Item_func_longfromgeohash, 1)},
    {"ST_LONGITUDE", SQL_FACTORY(Longitude_instantiator)},
    {"ST_MAKEENVELOPE", SQL_FN(Item_func_make_envelope, 2)},
    {"ST_MLINEFROMTEXT", SQL_FACTORY(Mlinefromtext_instantiator)},
    {"ST_MLINEFROMWKB", SQL_FACTORY(Mlinefromwkb_instantiator)},
    {"ST_MPOINTFROMTEXT", SQL_FACTORY(Mpointfromtext_instantiator)},
    {"ST_MPOINTFROMWKB", SQL_FACTORY(Mpointfromwkb_instantiator)},
    {"ST_MPOLYFROMTEXT", SQL_FACTORY(Mpolyfromtext_instantiator)},
    {"ST_MPOLYFROMWKB", SQL_FACTORY(Mpolyfromwkb_instantiator)},
    {"ST_MULTILINESTRINGFROMTEXT",
     SQL_FACTORY(Multilinestringfromtext_instantiator)},
    {"ST_MULTILINESTRINGFROMWKB",
     SQL_FACTORY(Multilinestringfromwkb_instantiator)},
    {"ST_MULTIPOINTFROMTEXT", SQL_FACTORY(Multipointfromtext_instantiator)},
    {"ST_MULTIPOINTFROMWKB", SQL_FACTORY(Multipointfromwkb_instantiator)},
    {"ST_MULTIPOLYGONFROMTEXT", SQL_FACTORY(Multipolygonfromtext_instantiator)},
    {"ST_MULTIPOLYGONFROMWKB", SQL_FACTORY(Multipolygonfromwkb_instantiator)},
    {"ST_NUMGEOMETRIES", SQL_FN(Item_func_numgeometries, 1)},
    {"ST_NUMINTERIORRING", SQL_FN(Item_func_numinteriorring, 1)},
    {"ST_NUMINTERIORRINGS", SQL_FN(Item_func_numinteriorring, 1)},
    {"ST_NUMPOINTS", SQL_FN(Item_func_numpoints, 1)},
    {"ST_OVERLAPS", SQL_FN(Item_func_st_overlaps, 2)},
    {"ST_POINTFROMGEOHASH", SQL_FN(Item_func_pointfromgeohash, 2)},
    {"ST_POINTFROMTEXT", SQL_FACTORY(Pointfromtext_instantiator)},
    {"ST_POINTFROMWKB", SQL_FACTORY(Pointfromwkb_instantiator)},
    {"ST_POINTN", SQL_FACTORY(Sp_pointn_instantiator)},
    {"ST_POLYFROMTEXT", SQL_FACTORY(Polyfromtext_instantiator)},
    {"ST_POLYFROMWKB", SQL_FACTORY(Polyfromwkb_instantiator)},
    {"ST_POLYGONFROMTEXT", SQL_FACTORY(Polygonfromtext_instantiator)},
    {"ST_POLYGONFROMWKB", SQL_FACTORY(Polygonfromwkb_instantiator)},
    {"ST_SIMPLIFY", SQL_FN(Item_func_st_simplify, 2)},
    {"ST_SRID", SQL_FACTORY(Srid_instantiator)},
    {"ST_STARTPOINT", SQL_FACTORY(Startpoint_instantiator)},
    {"ST_SYMDIFFERENCE", SQL_FN(Item_func_st_symdifference, 2)},
    {"ST_SWAPXY", SQL_FN(Item_func_swap_xy, 1)},
    {"ST_TOUCHES", SQL_FN(Item_func_st_touches, 2)},
    {"ST_TRANSFORM", SQL_FN(Item_func_st_transform, 2)},
    {"ST_UNION", SQL_FN(Item_func_st_union, 2)},
    {"ST_VALIDATE", SQL_FN(Item_func_validate, 1)},
    {"ST_WITHIN", SQL_FN(Item_func_st_within, 2)},
    {"ST_X", SQL_FACTORY(X_instantiator)},
    {"ST_Y", SQL_FACTORY(Y_instantiator)},
    {"SUBSTRING_INDEX", SQL_FN(Item_func_substr_index, 3)},
    {"SUBTIME", SQL_FACTORY(Subtime_instantiator)},
    {"TAN", SQL_FN(Item_func_tan, 1)},
    {"TIMEDIFF", SQL_FN(Item_func_timediff, 2)},
    {"TIME_FORMAT", SQL_FACTORY(Time_format_instantiator)},
    {"TIME_TO_SEC", SQL_FN(Item_func_time_to_sec, 1)},
    {"TO_BASE64", SQL_FN(Item_func_to_base64, 1)},
    {"TO_DAYS", SQL_FN(Item_func_to_days, 1)},
    {"TO_SECONDS", SQL_FN(Item_func_to_seconds, 1)},
    {"UCASE", SQL_FN(Item_func_upper, 1)},
    {"UNCOMPRESS", SQL_FN(Item_func_uncompress, 1)},
    {"UNCOMPRESSED_LENGTH", SQL_FN(Item_func_uncompressed_length, 1)},
    {"UNHEX", SQL_FN(Item_func_unhex, 1)},
    {"UNIX_TIMESTAMP", SQL_FN_V(Item_func_unix_timestamp, 0, 1)},
    {"UPDATEXML", SQL_FN(Item_func_xml_update, 3)},
    {"UPPER", SQL_FN(Item_func_upper, 1)},
    {"UUID", SQL_FN(Item_func_uuid, 0)},
    {"UUID_SHORT", SQL_FN(Item_func_uuid_short, 0)},
    {"UUID_TO_BIN", SQL_FN_V(Item_func_uuid_to_bin, 1, 2)},
    {"VALIDATE_PASSWORD_STRENGTH",
     SQL_FN(Item_func_validate_password_strength, 1)},
    {"VERSION", SQL_FN(Item_func_version, 0)},
    {"WEEKDAY", SQL_FACTORY(Weekday_instantiator)},
    {"WEEKOFYEAR", SQL_FACTORY(Weekofyear_instantiator)},
    {"YEARWEEK", SQL_FACTORY(Yearweek_instantiator)},
    {"GET_DD_COLUMN_PRIVILEGES",
     SQL_FN_INTERNAL(Item_func_get_dd_column_privileges, 3)},
    {"GET_DD_INDEX_SUB_PART_LENGTH",
     SQL_FN_LIST_INTERNAL(Item_func_get_dd_index_sub_part_length, 5)},
    {"GET_DD_CREATE_OPTIONS",
     SQL_FN_INTERNAL(Item_func_get_dd_create_options, 3)},
    {"GET_DD_TABLESPACE_PRIVATE_DATA",
     SQL_FN_INTERNAL(Item_func_get_dd_tablespace_private_data, 2)},
    {"GET_DD_INDEX_PRIVATE_DATA",
     SQL_FN_INTERNAL(Item_func_get_dd_index_private_data, 2)},
    {"INTERNAL_DD_CHAR_LENGTH",
     SQL_FN_INTERNAL(Item_func_internal_dd_char_length, 4)},
    {"CAN_ACCESS_DATABASE", SQL_FN_INTERNAL(Item_func_can_access_database, 1)},
    {"CAN_ACCESS_TABLE", SQL_FN_INTERNAL(Item_func_can_access_table, 2)},
    {"CAN_ACCESS_COLUMN", SQL_FN_INTERNAL(Item_func_can_access_column, 3)},
    {"CAN_ACCESS_VIEW", SQL_FN_INTERNAL(Item_func_can_access_view, 4)},
    {"CAN_ACCESS_TRIGGER", SQL_FN_INTERNAL(Item_func_can_access_trigger, 2)},
    {"CAN_ACCESS_ROUTINE",
     SQL_FN_LIST_INTERNAL(Item_func_can_access_routine, 5)},
    {"CAN_ACCESS_EVENT", SQL_FN_INTERNAL(Item_func_can_access_event, 1)},
    {"ICU_VERSION", SQL_FN(Item_func_icu_version, 0)},
    {"CAN_ACCESS_RESOURCE_GROUP",
     SQL_FN_INTERNAL(Item_func_can_access_resource_group, 1)},
    {"CONVERT_CPU_ID_MASK", SQL_FN_INTERNAL(Item_func_convert_cpu_id_mask, 1)},
    {"IS_VISIBLE_DD_OBJECT",
     SQL_FN_INTERNAL_V(Item_func_is_visible_dd_object, 1, 2)},
    {"INTERNAL_TABLE_ROWS",
     SQL_FN_LIST_INTERNAL_V(Item_func_internal_table_rows, 8, 9)},
    {"INTERNAL_AVG_ROW_LENGTH",
     SQL_FN_LIST_INTERNAL_V(Item_func_internal_avg_row_length, 8, 9)},
    {"INTERNAL_DATA_LENGTH",
     SQL_FN_LIST_INTERNAL_V(Item_func_internal_data_length, 8, 9)},
    {"INTERNAL_MAX_DATA_LENGTH",
     SQL_FN_LIST_INTERNAL_V(Item_func_internal_max_data_length, 8, 9)},
    {"INTERNAL_INDEX_LENGTH",
     SQL_FN_LIST_INTERNAL_V(Item_func_internal_index_length, 8, 9)},
    {"INTERNAL_DATA_FREE",
     SQL_FN_LIST_INTERNAL_V(Item_func_internal_data_free, 8, 9)},
    {"INTERNAL_AUTO_INCREMENT",
     SQL_FN_LIST_INTERNAL_V(Item_func_internal_auto_increment, 9, 10)},
    {"INTERNAL_CHECKSUM",
     SQL_FN_LIST_INTERNAL_V(Item_func_internal_checksum, 8, 9)},
    {"INTERNAL_UPDATE_TIME",
     SQL_FN_LIST_INTERNAL_V(Item_func_internal_update_time, 8, 9)},
    {"INTERNAL_CHECK_TIME",
     SQL_FN_LIST_INTERNAL_V(Item_func_internal_check_time, 8, 9)},
    {"INTERNAL_KEYS_DISABLED",
     SQL_FN_INTERNAL(Item_func_internal_keys_disabled, 1)},
    {"INTERNAL_INDEX_COLUMN_CARDINALITY",
     SQL_FN_LIST_INTERNAL(Item_func_internal_index_column_cardinality, 11)},
    {"INTERNAL_GET_COMMENT_OR_ERROR",
     SQL_FN_LIST_INTERNAL(Item_func_internal_get_comment_or_error, 5)},
    {"INTERNAL_GET_VIEW_WARNING_OR_ERROR",
     SQL_FN_LIST_INTERNAL(Item_func_internal_get_view_warning_or_error, 4)},
    {"INTERNAL_GET_PARTITION_NODEGROUP",
     SQL_FN_INTERNAL(Item_func_get_partition_nodegroup, 1)},
    {"INTERNAL_TABLESPACE_ID",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_id, 4)},
    {"INTERNAL_TABLESPACE_TYPE",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_type, 4)},
    {"INTERNAL_TABLESPACE_LOGFILE_GROUP_NAME",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_logfile_group_name, 4)},
    {"INTERNAL_TABLESPACE_LOGFILE_GROUP_NUMBER",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_logfile_group_number, 4)},
    {"INTERNAL_TABLESPACE_FREE_EXTENTS",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_free_extents, 4)},
    {"INTERNAL_TABLESPACE_TOTAL_EXTENTS",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_total_extents, 4)},
    {"INTERNAL_TABLESPACE_EXTENT_SIZE",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_extent_size, 4)},
    {"INTERNAL_TABLESPACE_INITIAL_SIZE",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_initial_size, 4)},
    {"INTERNAL_TABLESPACE_MAXIMUM_SIZE",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_maximum_size, 4)},
    {"INTERNAL_TABLESPACE_AUTOEXTEND_SIZE",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_autoextend_size, 4)},
    {"INTERNAL_TABLESPACE_VERSION",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_version, 4)},
    {"INTERNAL_TABLESPACE_ROW_FORMAT",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_row_format, 4)},
    {"INTERNAL_TABLESPACE_DATA_FREE",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_data_free, 4)},
    {"INTERNAL_TABLESPACE_STATUS",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_status, 4)},
    {"INTERNAL_TABLESPACE_EXTRA",
     SQL_FN_INTERNAL(Item_func_internal_tablespace_extra, 4)},
    {"GET_DD_PROPERTY_KEY_VALUE",
     SQL_FN_INTERNAL(Item_func_get_dd_property_key_value, 2)},
    {"REMOVE_DD_PROPERTY_KEY",
     SQL_FN_INTERNAL(Item_func_remove_dd_property_key, 2)},
    {"CONVERT_INTERVAL_TO_USER_INTERVAL",
     SQL_FN_INTERNAL(Item_func_convert_interval_to_user_interval, 2)},
    {"INTERNAL_GET_DD_COLUMN_EXTRA",
     SQL_FN_LIST_INTERNAL(Item_func_internal_get_dd_column_extra, 6)},
    {"INTERNAL_GET_USERNAME",
     SQL_FN_LIST_INTERNAL_V(Item_func_internal_get_username, 0, 1)},
    {"INTERNAL_GET_HOSTNAME",
     SQL_FN_LIST_INTERNAL_V(Item_func_internal_get_hostname, 0, 1)},
    {"INTERNAL_GET_ENABLED_ROLE_JSON",
     SQL_FN_INTERNAL(Item_func_internal_get_enabled_role_json, 0)},
    {"INTERNAL_GET_MANDATORY_ROLES_JSON",
     SQL_FN_INTERNAL(Item_func_internal_get_mandatory_roles_json, 0)},
    {"INTERNAL_IS_MANDATORY_ROLE",
     SQL_FN_INTERNAL(Item_func_internal_is_mandatory_role, 2)},
    {"INTERNAL_IS_ENABLED_ROLE",
     SQL_FN_INTERNAL(Item_func_internal_is_enabled_role, 2)}};

using Native_functions_hash = std::unordered_map<std::string, Create_func *>;
static const Native_functions_hash *native_functions_hash;

bool item_create_init() {
  try {
    native_functions_hash =
        new Native_functions_hash(std::begin(func_array), std::end(func_array));
  } catch (...) {
    handle_std_exception("item_create_init");
    return true;
  }
  return false;
}

void item_create_cleanup() { delete native_functions_hash; }

Create_func *find_native_function_builder(const LEX_STRING &lex_name) {
  try {
    std::string name(lex_name.str, lex_name.length);
    for (auto &it : name) it = std::toupper(it);

    auto entry = native_functions_hash->find(name);
    if (entry == native_functions_hash->end()) return nullptr;
    return entry->second;
  } catch (...) {
    handle_std_exception("find_native_function_builder");
    return nullptr;
  }
}

Create_qfunc *find_qualified_function_builder(THD *) {
  return &Create_sp_func::s_singleton;
}

Item *create_func_cast(THD *thd, const POS &pos, Item *a,
                       Cast_target cast_target, const CHARSET_INFO *cs) {
  Cast_type type;
  type.target = cast_target;
  type.charset = cs;
  type.length = nullptr;
  type.dec = nullptr;
  return create_func_cast(thd, pos, a, &type);
}

/**
  Validates a cast target type and extracts the specified length and precision
  of the target type.

  @param thd        thread handler
  @param pos        the location of the expression
  @param arg        the value to cast
  @param cast_type  the target type of the cast
  @param as_array   true if the target type is an array type
  @param[out] length     gets set to the maximum length of the target type
  @param[out] precision  gets set to the precision of the target type
  @return true on error, false on success
*/
static bool validate_cast_type_and_extract_length(
    const THD *thd, const POS &pos, Item *arg, const Cast_type &cast_type,
    bool as_array, int64_t *length, uint *precision) {
  // earlier syntax error detected
  if (arg == nullptr) return true;

  if (as_array) {
    // Disallow arrays in stored routines.
    if (thd->lex->get_sp_current_parsing_ctx()) {
      my_error(ER_WRONG_USAGE, MYF(0), "CAST( .. AS .. ARRAY)",
               "stored routines");
      return true;
    }

    /*
      Multi-valued index currently only supports two character sets: binary for
      BINARY(x) keys and my_charset_utf8mb4_0900_bin for CHAR(x) keys. The
      latter one is because it's closest to binary in terms of sort order and
      doesn't pad spaces. This is important because JSON treats e.g. "abc" and
      "abc " as different values and a space padding charset will cause
      inconsistent key handling.
    */
    if (cast_type.charset != nullptr && cast_type.charset != &my_charset_bin) {
      my_error(ER_NOT_SUPPORTED_YET, MYF(0),
               "specifying charset for multi-valued index");
      return true;
    }
  }

  *length = 0;
  *precision = 0;

  const char *const c_len = cast_type.length;
  const char *const c_dec = cast_type.dec;

  switch (cast_type.target) {
    case ITEM_CAST_SIGNED_INT:
    case ITEM_CAST_UNSIGNED_INT:
    case ITEM_CAST_DATE:
      return false;
    case ITEM_CAST_TIME:
    case ITEM_CAST_DATETIME: {
      uint dec = c_dec ? strtoul(c_dec, nullptr, 10) : 0;
      if (dec > DATETIME_MAX_DECIMALS) {
        my_error(ER_TOO_BIG_PRECISION, MYF(0), dec, "CAST",
                 DATETIME_MAX_DECIMALS);
        return true;
      }
      *precision = dec;
      return false;
    }
    case ITEM_CAST_DECIMAL: {
      ulong len = 0;
      uint dec = 0;

      if (c_len) {
        ulong decoded_size;
        errno = 0;
        decoded_size = strtoul(c_len, nullptr, 10);
        if (errno != 0) {
          StringBuffer<192> buff(pos.cpp.start, pos.cpp.length(),
                                 system_charset_info);
          my_error(ER_TOO_BIG_PRECISION, MYF(0), INT_MAX, buff.c_ptr_safe(),
                   static_cast<ulong>(DECIMAL_MAX_PRECISION));
          return true;
        }
        len = decoded_size;
      }

      if (c_dec) {
        ulong decoded_size;
        errno = 0;
        decoded_size = strtoul(c_dec, nullptr, 10);
        if ((errno != 0) || (decoded_size > UINT_MAX)) {
          // The parser rejects scale values above INT32_MAX, so this error path
          // is never taken.
          /* purecov: begin inspected */
          StringBuffer<192> buff(pos.cpp.start, pos.cpp.length(),
                                 system_charset_info);
          my_error(ER_TOO_BIG_SCALE, MYF(0), INT_MAX, buff.c_ptr_safe(),
                   static_cast<ulong>(DECIMAL_MAX_SCALE));
          return true;
          /* purecov: end */
        }
        dec = decoded_size;
      }
      my_decimal_trim(&len, &dec);
      if (len < dec) {
        my_error(ER_M_BIGGER_THAN_D, MYF(0), "");
        return true;
      }
      if (len > DECIMAL_MAX_PRECISION) {
        StringBuffer<192> buff(pos.cpp.start, pos.cpp.length(),
                               system_charset_info);
        my_error(ER_TOO_BIG_PRECISION, MYF(0), static_cast<int>(len),
                 buff.c_ptr_safe(), static_cast<ulong>(DECIMAL_MAX_PRECISION));
        return true;
      }
      if (dec > DECIMAL_MAX_SCALE) {
        StringBuffer<192> buff(pos.cpp.start, pos.cpp.length(),
                               system_charset_info);
        my_error(ER_TOO_BIG_SCALE, MYF(0), dec, buff.c_ptr_safe(),
                 static_cast<ulong>(DECIMAL_MAX_SCALE));
        return true;
      }
      *length = len;
      *precision = dec;
      return false;
    }
    case ITEM_CAST_CHAR: {
      longlong len = -1;
      if (c_len) {
        int error;
        len = my_strtoll10(c_len, nullptr, &error);
        if ((error != 0) || (len > MAX_FIELD_BLOBLENGTH)) {
          my_error(ER_TOO_BIG_DISPLAYWIDTH, MYF(0), "cast as char",
                   MAX_FIELD_BLOBLENGTH);
          return true;
        }
      }
      if (as_array && (len == -1 || len > CONVERT_IF_BIGGER_TO_BLOB)) {
        my_error(ER_NOT_SUPPORTED_YET, MYF(0),
                 "CAST-ing data to array of char/binary BLOBs");
        return true;
      }
      *length = len;
      return false;
    }
    case ITEM_CAST_DOUBLE:
      if (as_array) {
        my_error(ER_NOT_SUPPORTED_YET, MYF(0),
                 "CAST-ing data to array of DOUBLE");
        return true;
      }
      return false;
    case ITEM_CAST_FLOAT: {
      if (as_array) {
        my_error(ER_NOT_SUPPORTED_YET, MYF(0),
                 "CAST-ing data to array of FLOAT");
        return true;
      }

      ulong decoded_size = 0;

      // Check if binary precision is specified
      if (c_len != nullptr) {
        errno = 0;
        decoded_size = strtoul(c_len, nullptr, 10);
        if (errno != 0 || decoded_size > PRECISION_FOR_DOUBLE) {
          my_error(ER_TOO_BIG_PRECISION, MYF(0), decoded_size, "CAST",
                   PRECISION_FOR_DOUBLE);
          return true;
        }
      }
      *length = decoded_size;
      return false;
    }
    case ITEM_CAST_JSON:
      if (as_array) {
        my_error(ER_NOT_SUPPORTED_YET, MYF(0),
                 "CAST-ing data to array of JSON");
        return true;
      }
      return false;
  }
  /* purecov: begin deadcode */
  DBUG_ASSERT(false);
  return true;
  /* purecov: end */
}

Item *create_func_cast(THD *thd, const POS &pos, Item *arg,
                       const Cast_type *type, bool as_array) {
  int64_t length = 0;
  unsigned precision = 0;
  if (validate_cast_type_and_extract_length(thd, pos, arg, *type, as_array,
                                            &length, &precision))
    return nullptr;

  if (as_array)
    return new (thd->mem_root) Item_func_array_cast(
        pos, arg, type->target, length, precision, type->charset);

  switch (type->target) {
    case ITEM_CAST_SIGNED_INT:
      return new (thd->mem_root) Item_typecast_signed(pos, arg);
    case ITEM_CAST_UNSIGNED_INT:
      return new (thd->mem_root) Item_typecast_unsigned(pos, arg);
    case ITEM_CAST_DATE:
      return new (thd->mem_root) Item_typecast_date(pos, arg);
    case ITEM_CAST_TIME:
      return new (thd->mem_root) Item_typecast_time(pos, arg, precision);
    case ITEM_CAST_DATETIME:
      return new (thd->mem_root) Item_typecast_datetime(pos, arg, precision);
    case ITEM_CAST_DECIMAL:
      return new (thd->mem_root)
          Item_typecast_decimal(pos, arg, length, precision);
    case ITEM_CAST_CHAR: {
      const CHARSET_INFO *cs = type->charset;
      if (cs == nullptr) cs = thd->variables.collation_connection;
      return new (thd->mem_root) Item_typecast_char(pos, arg, length, cs);
    }
    case ITEM_CAST_JSON:
      return new (thd->mem_root) Item_typecast_json(thd, pos, arg);
    case ITEM_CAST_FLOAT:
      return new (thd->mem_root) Item_typecast_real(
          pos, arg, /*as_double=*/(length > PRECISION_FOR_FLOAT));
    case ITEM_CAST_DOUBLE:
      return new (thd->mem_root)
          Item_typecast_real(pos, arg, /*as_double=*/true);
  }

  /* purecov: begin deadcode */
  DBUG_ASSERT(false);
  return nullptr;
  /* purecov: end */
}

/**
  Builder for datetime literals:
    TIME'00:00:00', DATE'2001-01-01', TIMESTAMP'2001-01-01 00:00:00'.
  @param thd          The current thread
  @param str          Character literal
  @param length       Length of str
  @param cs           Character set of str
  @param type         Type of literal (TIME, DATE or DATETIME)
  @param send_error   Whether to generate an error on failure
*/

Item *create_temporal_literal(THD *thd, const char *str, size_t length,
                              const CHARSET_INFO *cs, enum_field_types type,
                              bool send_error) {
  MYSQL_TIME_STATUS status;
  MYSQL_TIME ltime;
  Item *item = nullptr;
  my_time_flags_t flags = TIME_FUZZY_DATE;
  if (thd->variables.sql_mode & MODE_NO_ZERO_IN_DATE)
    flags |= TIME_NO_ZERO_IN_DATE;
  if (thd->variables.sql_mode & MODE_NO_ZERO_DATE) flags |= TIME_NO_ZERO_DATE;

  if (thd->variables.sql_mode & MODE_INVALID_DATES) flags |= TIME_INVALID_DATES;

  switch (type) {
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_NEWDATE:
      if (!propagate_datetime_overflow(
              thd, &status.warnings,
              str_to_datetime(cs, str, length, &ltime, flags, &status)) &&
          ltime.time_type == MYSQL_TIMESTAMP_DATE && !status.warnings)
        item = new (thd->mem_root) Item_date_literal(&ltime);
      break;
    case MYSQL_TYPE_DATETIME:
      if (!propagate_datetime_overflow(
              thd, &status.warnings,
              str_to_datetime(cs, str, length, &ltime, flags, &status)) &&
          (ltime.time_type == MYSQL_TIMESTAMP_DATETIME ||
           ltime.time_type == MYSQL_TIMESTAMP_DATETIME_TZ) &&
          !status.warnings) {
        adjust_time_zone_displacement(thd->time_zone(), &ltime);
        item = new (thd->mem_root) Item_datetime_literal(
            &ltime, status.fractional_digits, thd->time_zone());
      }
      break;
    case MYSQL_TYPE_TIME:
      if (!str_to_time(cs, str, length, &ltime, 0, &status) &&
          ltime.time_type == MYSQL_TIMESTAMP_TIME && !status.warnings)
        item = new (thd->mem_root)
            Item_time_literal(&ltime, status.fractional_digits);
      break;
    default:
      DBUG_ASSERT(0);
  }

  if (item) return item;

  if (send_error) {
    const char *typestr = (type == MYSQL_TYPE_DATE)
                              ? "DATE"
                              : (type == MYSQL_TYPE_TIME) ? "TIME" : "DATETIME";
    ErrConvString err(str, length, thd->variables.character_set_client);
    my_error(ER_WRONG_VALUE, MYF(0), typestr, err.ptr());
  }
  return nullptr;
}

/**
  @} (end of group GROUP_PARSER)
*/
