/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_PROF_TRANS_DATA_H_
#define incl_HPHP_PROF_TRANS_DATA_H_

#include <vector>

#include "hphp/util/base.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/runtime-type.h"
#include "hphp/runtime/vm/jit/region-selection.h"

namespace HPHP {
namespace JIT {

using Transl::TransID;
using Transl::TransKind;
using Transl::Tracelet;

/**
 * A simple class of a growable number of profiling counters with
 * fixed addresses, suitable for being incremented from the TC.
 */
template<typename T>
class ProfCounters {
 public:
  explicit ProfCounters(T initVal)
      : m_initVal(initVal)
    {}

  ProfCounters(const ProfCounters&)            = delete;
  ProfCounters& operator=(const ProfCounters&) = delete;

  ~ProfCounters() {
    for (size_t i = 0; i < m_chunks.size(); i++) {
      free(m_chunks[i]);
    }
  }

  T  get(uint32_t id) const;
  T* getAddr(uint32_t id);

 private:
  static const uint32_t kCountersPerChunk = 2 * 1024 * 1024 / sizeof(T);

  T                     m_initVal;
  vector<T*>            m_chunks;
};


/**
 * A profiling record kept for each translation in JitPGO mode.
 */
class ProfTransRec {
 public:
  ProfTransRec(TransID id, TransKind kind, Offset lastBcOff, const SrcKey& sk,
               RegionDesc::BlockPtr block);
  ProfTransRec(TransID id, TransKind kind, const SrcKey& sk);

  TransID              transId()    const;
  TransKind            kind()       const;
  SrcKey               srcKey()     const;
  Offset               startBcOff() const;
  Offset               lastBcOff()  const;
  Func*                func()       const;
  FuncId               funcId()     const;
  RegionDesc::BlockPtr block()      const;

 private:
  TransID              m_id;  // sequential ID of the assiciated translation
  TransKind            m_kind;
  Offset               m_lastBcOff;  // offset of the last bytecode instr
  RegionDesc::BlockPtr m_block;
  SrcKey               m_sk;
};

typedef std::unique_ptr<ProfTransRec> ProfTransRecPtr;

/**
 * ProfData encapsulates the profiling data kept by the JIT.
 */
class ProfData {
public:
  ProfData();

  ProfData(const ProfData&)            = delete;
  ProfData& operator=(const ProfData&) = delete;

  TransID                 numTrans()                  const;
  TransID                 curTransID()                const;

  SrcKey                  transSrcKey(TransID id)     const;
  Offset                  transStartBcOff(TransID id) const;
  Offset                  transLastBcOff(TransID id)  const;
  Op*                     transLastInstr(TransID id)  const;
  Offset                  transStopBcOff(TransID id)  const;
  FuncId                  transFuncId(TransID id)     const;
  Func*                   transFunc(TransID id)       const;
  RegionDesc::BlockPtr    transBlock(TransID id)      const;
  TransKind               transKind(TransID id)       const;
  int64_t                 transCounter(TransID id)    const;
  int64_t*                transCounterAddr(TransID id);

  TransID                 addTrans(const Tracelet& tracelet, TransKind kind,
                                   const PostConditions& pconds);
  TransID                 addTransPrologue(const SrcKey& sk);
  TransID                 addTransAnchor(const SrcKey& sk);

  bool                    optimized(const SrcKey& sk) const;
  void                    setOptimized(const SrcKey& sk);

private:
  uint32_t                m_numTrans;
  vector<ProfTransRecPtr> m_transRecs;
  ProfCounters<int64_t>   m_counters;
  SrcKeySet               m_optimized;  // set of SrcKeys already optimized
};

} }

#endif
