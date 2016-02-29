/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_CODE_GEN_FIXUPS_H_
#define incl_HPHP_JIT_CODE_GEN_FIXUPS_H_

#include "hphp/runtime/vm/jit/alignment.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/growable-vector.h"

#include <map>
#include <vector>

namespace HPHP { namespace jit {

/*
 * CGMeta contains a variety of different metadata information that is
 * collected during code generation.
 */
struct CGMeta {
  void process(GrowableVector<IncomingBranch>* inProgressTailBranches);
  void process_only(GrowableVector<IncomingBranch>* inProgressTailBranches);
  bool empty() const;
  void clear();

  void setJmpTransID(TCA jmp, TransKind kind);

  std::vector<std::pair<TCA, Fixup>> fixups;
  std::vector<std::pair<CTCA, TCA>> catches;
  std::vector<std::pair<TCA,TransID>> jmpTransIDs;
  std::vector<TCA> reusedStubs;
  std::set<TCA> addressImmediates;
  std::set<TCA*> codePointers;
  std::vector<TransBCMapping> bcMap;
  std::multimap<TCA,std::pair<Alignment,AlignContext>> alignments;
  GrowableVector<IncomingBranch> inProgressTailJumps;
  LiteralMap literals;
  Annotations annotations;
};

}}

#endif
