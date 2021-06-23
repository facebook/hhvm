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

#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP { namespace jit { namespace tc {

struct RegionTranslator final : Translator {
  PostConditions pconds;
  Annotations annotations{};
  // A sequential per function index to identify optimized
  // translations in TRACE and StructuredLog output (in particular to
  // make it possible to cross reference between the two).
  int optIndex{0};
  RegionDescPtr region{nullptr};
  bool hasLoop{false};
  SBInvOffset spOff{};
  jit::vector<RegionContext::LiveType> liveTypes{};
  int prevNumTranslations{-1};
  GrowableVector<IncomingBranch> tailBranches;

  explicit RegionTranslator(SrcKey sk, TransKind kind = TransKind::Invalid)
    : Translator(sk, kind)
  {}

  RegionContext regionContext() {
    RegionContext ret{sk, spOff};
    ret.liveTypes = liveTypes;
    return ret;
  }
  void computeKind() override;
  Optional<TranslationResult> getCached() override;
  void resetCached() override;
  void setCachedForProcessFail() override;
  void smashBackup() override {}
  Annotations* getAnnotations() override { return &annotations; }
  void gen() override;
  void publishMetaImpl() override;
  void publishCodeImpl() override;
};

}}}
