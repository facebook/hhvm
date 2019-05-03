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
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/rds-local.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/type-scan.h"

#include <folly/container/F14Map.h>


namespace HPHP { namespace arrprov {

namespace {

RDS_LOCAL(ArrayProvenanceTable, rl_array_provenance);

/*
 * Flush the table after each request since none of the ArrayData*s will be
 * valid anymore
 */
InitFiniNode flushTable([]{
    if (!RuntimeOption::EvalLogArrayProvenance) return;
    rl_array_provenance->tags.clear();
  }, InitFiniNode::When::RequestFini);

} // anonymous namespace

namespace unchecked {

bool arrayWantsTag(const ArrayData* ad) {
  if (!ad->isRefCounted()) return false;
  auto const kind = ad->kind();
  return kind == ArrayData::ArrayKind::kVecKind ||
         kind == ArrayData::ArrayKind::kDictKind;
}

void setTag(ArrayData* ad, const Tag& tag) {
  if (!arrayWantsTag(ad)) return;
  assertx(!ad->empty());
  assertx(!getTag(ad));
  ad->markHasProvenanceData();
  rl_array_provenance->tags[ad] = tag;
}

folly::Optional<Tag> getTag(const ArrayData* ad) {
  if (!ad->hasProvenanceData()) return {};
  return rl_array_provenance->tags[ad];
}

void clearTag(const ArrayData* ad) {
  assertx(ad->isRefCounted());
  if (!arrayWantsTag(ad)) return;
  rl_array_provenance->tags.erase(ad);
}

}

Tag tagFromProgramCounter() {
  VMRegAnchor _;
  auto const unit = vmfp()->m_func->unit();
  auto const pc = vmpc();
  auto const filename = unit->filepath();
  auto const line = unit->getLineNumber(unit->offsetOf(pc));
  return Tag{filename, line};
}

}} // namespace HPHP::arrprov
