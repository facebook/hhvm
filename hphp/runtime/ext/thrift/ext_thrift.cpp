/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/thrift/ext_thrift.h"

namespace HPHP { namespace thrift {

static class ThriftExtension final : public Extension {
public:
  ThriftExtension() : Extension("thrift_protocol", NO_EXTENSION_VERSION_YET) {}
  void moduleInit() override {
    HHVM_FE(thrift_protocol_write_binary);
    HHVM_FE(thrift_protocol_read_binary);
    HHVM_FE(thrift_protocol_read_binary_struct);
    HHVM_FE(thrift_protocol_set_compact_version);
    HHVM_FE(thrift_protocol_write_compact);
    HHVM_FE(thrift_protocol_read_compact);
    HHVM_FE(thrift_protocol_read_compact_struct);

    loadSystemlib("thrift");
  }
} s_thrift_extension;

}}
