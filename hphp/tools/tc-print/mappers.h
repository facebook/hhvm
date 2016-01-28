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

#ifndef incl_HPHP_ADDR_TO_BC_MAPPER_
#define incl_HPHP_ADDR_TO_BC_MAPPER_

#include <vector>
#include <string>

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/tools/tc-print/perf-events.h"
#include "hphp/tools/tc-print/offline-trans-data.h"
#include "hphp/tools/tc-print/offline-x86-code.h"
#include "hphp/tools/tc-print/repo-wrapper.h"

namespace HPHP { namespace jit {

typedef uint16_t ExtOpcode;

/* Utilities */

std::string extOpcodeToString(ExtOpcode eOpcode);
std::vector<std::pair<std::string, ExtOpcode> > getValidOpcodeNames();
ExtOpcode stringToExtOpcode(std::string s);
std::string tcRegionToString(TCRegion tcr);

/* AddrToBcMapper */

class AddrToBcMapper : public Mapper<TCA, ExtOpcode> {
  const OfflineTransData* transData;

public:
  explicit AddrToBcMapper(const OfflineTransData* _transData) :
    transData(_transData) {}

  folly::Optional<ExtOpcode> operator()(const TCA& addr);
};

/* AddrToTransMapper */

class AddrToTransMapper : public Mapper<TCA, TransID> {
  const OfflineTransData* tdata;

public:
  explicit AddrToTransMapper(const OfflineTransData* _tdata) : tdata(_tdata) {}

  folly::Optional<TransID> operator()(const TCA& addr) {
    always_assert(tdata);
    TransID tid = tdata->getTransContaining(addr);
    if (tid != INVALID_ID) return tid;
    return folly::none;
  }
};

/* AddrToTransFragmentMapper */

struct TransFragment {
  TransID  tid;
  TCA      aStart;
  TCA      acoldStart;
  TCA      afrozenStart;
  uint32_t aLen;
  uint32_t acoldLen;
  uint32_t afrozenLen;

  // Since it's going into an ordered map...
  bool operator<(const TransFragment& other) const {
    return aStart < other.aStart;
  }
};

class AddrToTransFragmentMapper : public Mapper<TCA, TransFragment> {
  const     OfflineTransData* tdata;
  ExtOpcode filterBy;

  TransFragment extractTransFragment(TCA addr, ExtOpcode opcode);

public:
  AddrToTransFragmentMapper(const OfflineTransData* _tdata,
                            ExtOpcode _filterBy) :
    tdata(_tdata), filterBy(_filterBy) {}

  folly::Optional<TransFragment> operator()(const TCA& addr);
};

/* TransToFuncMapper */

class TransToFuncMapper : public Mapper<TransID, FuncId> {
  const OfflineTransData* tdata;

public:
  explicit TransToFuncMapper(const OfflineTransData* _tdata) : tdata(_tdata) {}

  folly::Optional<FuncId> operator()(const TransID& tid) {
    always_assert(tdata);
    return tdata->getTransRec(tid)->src.funcID();
  }
};

} }

#endif
