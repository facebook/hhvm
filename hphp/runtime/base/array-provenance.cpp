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

#include "hphp/runtime/base/array-provenance.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/util/rds-local.h"
#include "hphp/util/type-scan.h"

#include <folly/AtomicHashMap.h>
#include <folly/container/F14Map.h>

namespace HPHP { namespace arrprov {

///////////////////////////////////////////////////////////////////////////////

namespace {

RDS_LOCAL(ArrayProvenanceTable, rl_array_provenance);
folly::F14FastMap<const ArrayData*, Tag> s_static_array_provenance;
std::mutex s_static_provenance_lock;

/*
 * Flush the table after each request since none of the ArrayData*s will be
 * valid anymore
 */
InitFiniNode flushTable([]{
    if (!RuntimeOption::EvalArrayProvenance) return;
    rl_array_provenance->tags.clear();
  }, InitFiniNode::When::RequestFini);

} // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

bool arrayWantsTag(const ArrayData* ad) {
  auto const kind = ad->kind();
  return kind == ArrayData::ArrayKind::kVecKind ||
         kind == ArrayData::ArrayKind::kDictKind;
}

void setTag(ArrayData* ad, const Tag& tag) {
  if (!arrayWantsTag(ad)) return;
  assertx(ad->hasExactlyOneRef() || !ad->empty());
  assertx(!getTag(ad));
  ad->markHasProvenanceData();
  if (ad->isRefCounted()) {
    rl_array_provenance->tags[ad] = tag;
  } else {
    std::lock_guard<std::mutex> g{s_static_provenance_lock};
    s_static_array_provenance[ad] = tag;
  }
}

folly::Optional<Tag> getTag(const ArrayData* ad) {
  if (!ad->hasProvenanceData()) return {};
  if (ad->isRefCounted()) {
    return rl_array_provenance->tags[ad];
  } else {
    std::lock_guard<std::mutex> g{s_static_provenance_lock};
    return s_static_array_provenance.find(ad)->second;
  }
}

void clearTag(const ArrayData* ad) {
  assertx(ad->isRefCounted());
  if (!arrayWantsTag(ad)) return;
  rl_array_provenance->tags.erase(ad);
}

TypedValue tagTV(TypedValue tv) {
  using namespace arrprov;

  assertx(RuntimeOption::EvalArrayProvenance);
  if (!tvWantsTag(tv)) return tv;

  auto ad = val(tv).parr;
  if (getTag(ad)) return tv;

  if (!ad->hasExactlyOneRef()) {
    ad = ad->copy();
    type(tv) = dt_with_rc(type(tv));
    val(tv).parr = ad;
  }
  setTag(ad, tagFromProgramCounter());
  return tv;
}

Tag tagFromProgramCounter() {
  auto const tag = fromLeaf(
    [&] (const ActRec* fp, Offset offset) {
      auto const unit = fp->unit();
      auto const filename = unit->filepath();
      auto const line = unit->getLineNumber(offset);
      return Tag { filename, line };
    },
    [] (const ActRec* fp) { return !fp->func()->isProvenanceSkipFrame(); }
  );
  assertx(tag.filename() != nullptr);
  return tag;
}

///////////////////////////////////////////////////////////////////////////////

}}
