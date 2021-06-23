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

#include "hphp/runtime/vm/jit/tc.h"

namespace HPHP { namespace jit { namespace tc {

struct PrologueTranslator final : Translator {
  // Although the srckey (sk member of translator) knows the func,
  // it holds it as a const Func* and won't work for the mutations
  // required for publishing.
  Func* func;
  int nPassed;
  PrologueTranslator(Func* func, int nPassed,
                     TransKind kind = TransKind::Invalid)
    : Translator(
        SrcKey{
          func,
          func->getEntryForNumArgs(paramIndexHelper(func, nPassed)),
          SrcKey::PrologueTag{}
        },
        kind
      )
    , func(func)
    , nPassed(nPassed)
  {}
  int paramIndex() const;
  Optional<TranslationResult> getCached() override;
  void resetCached() override;
  void setCachedForProcessFail() override;
  void smashBackup() override;
  static int paramIndexHelper(const Func*, int);
private:
  void computeKind() override;
  Annotations* getAnnotations() override { return nullptr; }
  void gen() override;
  void publishMetaImpl() override;
  void publishCodeImpl() override;
};

}}}
