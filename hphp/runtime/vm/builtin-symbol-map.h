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

#include "hphp/runtime/base/autoload-map.h"

#include "hphp/util/optional.h"

#include <string>

namespace HPHP::hackc {
  struct ExternalDeclProviderResult;
  struct DeclsAndBlob;
}

namespace HPHP::Native {

/**
 * Record all declarations in file `name` with source code `contents`.
 * @precondition: RuntimeOption::EvalEnableDecl is true
 * @precondition: `name` and `contents` represent PHP that is shipped with HHVM
 */
void registerBuiltinSymbols(const std::string& serialized_decls);


/**
 * Find the collection of decls that contains the decl with name `symbol` and
 * decl kind `kind`.
 * @postcondition: For all `r` returned, if `r.has_value()`, then one of the
 * decls in `r` *must* have the name represented by `symbol`
 */
Optional<hackc::ExternalDeclProviderResult> getBuiltinDecls(
  const StringData* symbol,
  AutoloadMap::KindOf kind
);

hphp_fast_set<const hackc::DeclsAndBlob*> getAllBuiltinDecls();

} // namespace HPHP::Native
