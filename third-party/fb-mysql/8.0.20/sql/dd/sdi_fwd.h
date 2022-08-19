/* Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_SDI_FWD_H_INCLUDED
#define DD_SDI_FWD_H_INCLUDED

#include <rapidjson/fwd.h>
/**
  @file
  @ingroup sdi
  This header provides @ref dd_rj_type_alias
*/

/**
  @defgroup dd_rj_type_alias Rapidjson Type Aliases
  @ingroup sdi

  Create type aliases for rapidjson template instantiations which will
  be used by (de)serialization code.

  @{
*/
namespace dd {
typedef rapidjson::UTF8<char> RJ_Encoding;
typedef rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> RJ_Allocator;
typedef rapidjson::GenericDocument<RJ_Encoding, RJ_Allocator,
                                   rapidjson::CrtAllocator>
    RJ_Document;
typedef rapidjson::GenericValue<RJ_Encoding, RJ_Allocator> RJ_Value;
typedef rapidjson::GenericStringBuffer<RJ_Encoding, rapidjson::CrtAllocator>
    RJ_StringBuffer;
typedef rapidjson::PrettyWriter<RJ_StringBuffer, RJ_Encoding, RJ_Encoding,
                                RJ_Allocator, 0>
    RJ_PrettyWriter;

using RJ_Writer = rapidjson::Writer<RJ_StringBuffer, RJ_Encoding, RJ_Encoding,
                                    RJ_Allocator, 0>;

/**
  Alias for the rapidjson Writer type to use in serialization.
  Can be changeed to RJ_PrettyWriter to get human-readable (but
  significatly larger) sdis.
*/
using Sdi_writer = RJ_Writer;

class Sdi_rcontext;
class Sdi_wcontext;
}  // namespace dd
/** @} */  // dd_rj_type_alias

#endif /* DD_SDI_FWD_H_INCLUDED */
