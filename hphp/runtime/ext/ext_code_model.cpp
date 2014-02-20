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

#include "ext_code_model.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {

static class CodeModelExtension : public Extension {
 public:
  CodeModelExtension() : Extension("code_model", NO_EXTENSION_VERSION_YET) { }
  virtual void moduleLoad(Hdf config) {
    HHVM_NAMED_FE(HH\\CodeModel\\get_code_model_for,
                  HHVM_FN(get_code_model_for)
    );
  }
} s_code_model_extension;

String HHVM_FUNCTION(get_code_model_for, String source) {
  return g_hphp_compiler_serialize_code_model_for(source, "HH\\CodeModel\\");
}

}
