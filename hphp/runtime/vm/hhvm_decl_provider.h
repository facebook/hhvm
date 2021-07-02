/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/hack/src/decl/cpp_ffi/decl_ffi_types.h"
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/type-string.h"

#include <map>
#include <memory>

namespace HPHP {

struct HhvmDeclProvider {
  HhvmDeclProvider() {}
  ~HhvmDeclProvider();
  HhvmDeclProvider(HhvmDeclProvider const&) = delete;
  HhvmDeclProvider& operator=(HhvmDeclProvider const&) = delete;

  hackc::decl::decls const* getDecl(HPHP::AutoloadMap::KindOf kind, char const* symbol);

 private:
  std::map<std::string, hackc::decl::decl_result> m_cache;
};

hackc::decl::decls const* hhvm_decl_provider_get_decl(void* provider, char const* symbol);
}
