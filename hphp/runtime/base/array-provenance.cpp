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
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/util/rds-local.h"
#include "hphp/util/type-scan.h"
#include "hphp/util/type-traits.h"

#include <folly/AtomicHashMap.h>
#include <folly/container/F14Map.h>
#include <folly/Format.h>

namespace HPHP { namespace arrprov {

////////////////////////////////////////////////////////////////////////////////

std::string Tag::toString() const {
  assertx(m_filename);
  return folly::sformat("{}:{}", m_filename->slice(), m_line);
}

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

namespace {

template <bool allow_overwrite>
void setTagImpl(ArrayData* ad, const Tag& tag) {
  if (!arrayWantsTag(ad)) return;
  assertx(allow_overwrite || !getTag(ad));
  ad->markHasProvenanceData();
  if (ad->isRefCounted()) {
    rl_array_provenance->tags[ad] = tag;
  } else {
    std::lock_guard<std::mutex> g{s_static_provenance_lock};
    s_static_array_provenance[ad] = tag;
  }
}

} // namespace

void setTag(ArrayData* ad, const Tag& tag) {
  setTagImpl<false>(ad, tag);
}

void setTagReplace(ArrayData* ad, const Tag& tag) {
  setTagImpl<true>(ad, tag);
}

void setTagRecursive(ArrayData* ad, const Tag& tag) {
  assertx(RuntimeOption::EvalArrayProvenance);
  if (ad->empty()) return;
  if (!ad->isRefCounted()) return;

  if (arrayWantsTag(ad) &&
      !ad->hasProvenanceData()) {
    setTag(ad, tag);
  }

  IterateVNoInc(
    ad,
    [&](TypedValue tv) {
      if (!isArrayLikeType(tv.m_type)) return;
      setTagRecursive(tv.m_data.parr, tag);
    }
  );
}

folly::Optional<Tag> getTag(const ArrayData* ad) {
  if (!ad->hasProvenanceData()) return {};
  if (ad->isRefCounted()) {
    auto const iter = rl_array_provenance->tags.find(ad);
    assertx(iter != rl_array_provenance->tags.cend());
    assertx(iter->second.filename());
    return iter->second;
  } else {
    std::lock_guard<std::mutex> g{s_static_provenance_lock};
    auto const ret = s_static_array_provenance.find(ad)->second;
    assertx(ret.filename());
    return ret;
  }
}

void clearTag(const ArrayData* ad) {
  assertx(ad->isRefCounted());
  if (!arrayWantsTag(ad)) return;
  rl_array_provenance->tags.erase(ad);
}

namespace {

void tagTVImpl(TypedValue& tv, folly::Optional<Tag> tag) {
  assertx(RuntimeOption::EvalArrayProvenance);

  if (!tvWantsTag(tv)) return;

  auto ad = val(tv).parr;
  if (ad->hasProvenanceData()) return;

  if (!tag) tag = tagFromProgramCounter();
  if (!tag) return;

  if (!ad->hasExactlyOneRef()) {
    ad = ad->copy();

    TypedValue tmp;
    type(tmp) = dt_with_rc(type(tv));
    val(tmp).parr = ad;

    tvMove(tmp, tv);
  }
  setTag(ad, *tag);
}

}

TypedValue tagTV(TypedValue tv) {
  tagTVImpl(tv, folly::none);
  return tv;
}

TypedValue tagTVKnown(TypedValue tv, Tag tag) {
  tagTVImpl(tv, tag);
  return tv;
}

namespace {

template<typename AD, typename Copy>
typename maybe_const<AD, ArrayData, AD*>::type
makeEmptyImpl(AD* base, folly::Optional<Tag> tag, Copy&& copy) {
  assertx(base->empty());
  assertx(base->isStatic());
  assertx(arrayWantsTag(base));

  if (!tag) tag = tagFromProgramCounter();
  if (!tag) return base;

  auto ad = copy(base);
  assertx(ad);
  assertx(ad->hasExactlyOneRef());

  arrprov::setTagReplace(ad, *tag);
  ArrayData::GetScalarArray(&ad);

  return ad;
}

}

const ArrayData* makeEmptyArray(const ArrayData* base,
                                folly::Optional<Tag> tag) {
  return makeEmptyImpl(base, tag, [] (const ArrayData* ad) {
    return ad->copy();
  });
}

ArrayData* makeEmptyVec(folly::Optional<Tag> tag /* = folly::none */) {
  return makeEmptyImpl(
    staticEmptyVecArray(), tag,
    [] (const ArrayData* ad) { return PackedArray::CopyVec(ad); }
  );
}

ArrayData* makeEmptyDict(folly::Optional<Tag> tag /* = folly::none */) {
  return makeEmptyImpl(
    staticEmptyDictArray(), tag,
    [] (const ArrayData* ad) { return MixedArray::CopyDict(ad); }
  );
}

folly::Optional<Tag> tagFromProgramCounter() {
  VMRegAnchor _(VMRegAnchor::Soft);
  if (tl_regState != VMRegState::CLEAN || vmfp() == nullptr) {
    return folly::none;
  }

  auto const tag = fromLeaf(
    [&] (const ActRec* fp, Offset offset) -> folly::Optional<Tag> {
      auto const unit = fp->unit();
      auto const filename = unit->filepath();
      auto const line = unit->getLineNumber(offset);
      return Tag { filename, line };
    },
    [] (const ActRec* fp) {
      return !fp->func()->isProvenanceSkipFrame() &&
             !fp->func()->isCPPBuiltin();
    }
  );
  assertx(!tag || tag->filename() != nullptr);
  return tag;
}

///////////////////////////////////////////////////////////////////////////////

}}
