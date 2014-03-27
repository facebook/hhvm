/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/hh/ext_hh.h"

#include "hphp/runtime/base/file-repository.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
bool HHVM_FUNCTION(autoload_set_paths,
                   const Variant& map,
                   const String& root) {
  if (!map.isArray()) {
    return false;
  }
  return AutoloadHandler::s_instance->setMap(map.toCArrRef(), root);
}

bool HHVM_FUNCTION(could_include, const String& file) {
  struct stat s;
  return !Eval::resolveVmInclude(file.get(), "", &s).isNull();
}

static class HHExtension : public Extension {
 public:
  HHExtension(): Extension("hh", NO_EXTENSION_VERSION_YET) { }
  virtual void moduleInit() {
    HHVM_NAMED_FE(HH\\autoload_set_paths, HHVM_FN(autoload_set_paths));
    HHVM_NAMED_FE(HH\\could_include, HHVM_FN(could_include));
    loadSystemlib();
  }
} s_hh_extension;

///////////////////////////////////////////////////////////////////////////////
}
