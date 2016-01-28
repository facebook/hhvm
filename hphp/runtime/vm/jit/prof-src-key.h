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

#ifndef incl_HPHP_JIT_PROF_SRC_KEY_H_
#define incl_HPHP_JIT_PROF_SRC_KEY_H_

#include "hphp/runtime/vm/srckey.h"

#include <boost/operators.hpp>

namespace HPHP { namespace jit {

/**
 * A simple struct identifying a bytecode instruction in a given
 * profiling translation.
 */
struct ProfSrcKey : private boost::totally_ordered<ProfSrcKey> {
  TransID profTransId;
  SrcKey  srcKey;

  ProfSrcKey(TransID tid, SrcKey sk)
    : profTransId(tid)
    , srcKey(sk)
  { }

  bool operator==(const ProfSrcKey& other) const {
    return profTransId == other.profTransId && srcKey == other.srcKey;
  }

  bool operator<(const ProfSrcKey& other) const {
    return profTransId < other.profTransId ||
      (profTransId == other.profTransId && srcKey < other.srcKey);
  }

  struct Hasher;
};

struct ProfSrcKey::Hasher {
  size_t operator()(ProfSrcKey psk) const {
    return hash_int64_pair(psk.profTransId, psk.srcKey.toAtomicInt());
  }
};

typedef hphp_hash_set<ProfSrcKey, ProfSrcKey::Hasher> ProfSrcKeySet;

} }

#endif
