/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_EXT_THRIFT_H_
#define incl_HPHP_EXT_THRIFT_H_

#include "hphp/runtime/ext/extension.h"

namespace HPHP { namespace thrift {
///////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(thrift_protocol_write_binary,
                   const Object& transportobj,
                   const String& method_name,
                   int64_t msgtype,
                   const Object& request_struct,
                   int seqid,
                   bool strict_write,
                   bool oneway = false);

Object HHVM_FUNCTION(thrift_protocol_read_binary,
                     const Object& transportobj,
                     const String& obj_typename,
                     bool strict_read);

Variant HHVM_FUNCTION(thrift_protocol_read_binary_struct,
                      const Object& transportobj,
                      const String& obj_typename);

int64_t HHVM_FUNCTION(thrift_protocol_set_compact_version,
                      int version);

void HHVM_FUNCTION(thrift_protocol_write_compact,
                   const Object& transportobj,
                   const String& method_name,
                   int64_t msgtype,
                   const Object& request_struct,
                   int seqid,
                   bool oneway = false);

Variant HHVM_FUNCTION(thrift_protocol_read_compact,
                      const Object& transportobj,
                      const String& obj_typename);

Object HHVM_FUNCTION(thrift_protocol_read_compact_struct,
                     const Object& transportobj,
                     const String& obj_typename);

///////////////////////////////////////////////////////////////////////////////
}}
#endif // incl_HPHP_EXT_THRIFT_H_
