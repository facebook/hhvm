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

#include "hphp/runtime/base/apc-array.h"
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

#include <type_traits>

namespace HPHP { namespace arrprov {

////////////////////////////////////////////////////////////////////////////////

std::string Tag::toString() const {
  assertx(m_filename);
  return folly::sformat("{}:{}", m_filename->slice(), m_line);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

RDS_LOCAL(ArrayProvenanceTable, rl_array_provenance);
folly::F14FastMap<const void*, Tag> s_static_array_provenance;
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

namespace {

/*
 * Whether provenance for a given array should be request-local.
 *
 * True for refcounted request arrays, else false.
 */
bool wants_local_prov(const ArrayData* ad) { return ad->isRefCounted(); }
constexpr bool wants_local_prov(const APCArray* a) { return false; }

}

bool arrayWantsTag(const ArrayData* ad) {
  auto const kind = ad->kind();
  return kind == ArrayData::ArrayKind::kVecKind ||
         kind == ArrayData::ArrayKind::kDictKind;
}

bool arrayWantsTag(const APCArray* a) {
  return a->isVec() || a->isDict();
}

namespace {

template<typename A>
folly::Optional<Tag> getTagImpl(const A* a) {
  using ProvenanceTable = decltype(s_static_array_provenance);

  static_assert(std::is_same<
    ProvenanceTable,
    decltype(rl_array_provenance->tags)
  >::value, "Static and request-local provenance tables must share a type.");

  auto const get = [] (
    const A* a,
    const ProvenanceTable& tbl
  ) -> folly::Optional<Tag> {
    auto const it = tbl.find(a);
    if (it == tbl.cend()) return folly::none;
    assertx(it->second.filename() != nullptr);
    return it->second;
  };

  if (wants_local_prov(a)) {
    return get(a, rl_array_provenance->tags);
  } else {
    std::lock_guard<std::mutex> g{s_static_provenance_lock};
    return get(a, s_static_array_provenance);
  }
}

template<Mode mode, typename A>
bool setTagImpl(A* a, Tag tag) {
  if (!arrayWantsTag(a)) return false;
  assertx(mode == Mode::Emplace || !getTag(a));

  if (wants_local_prov(a)) {
    rl_array_provenance->tags[a] = tag;
  } else {
    std::lock_guard<std::mutex> g{s_static_provenance_lock};
    s_static_array_provenance[a] = tag;
  }
  return true;
}

template<typename A>
void clearTagImpl(const A* a) {
  if (!arrayWantsTag(a)) return;

  if (wants_local_prov(a)) {
    rl_array_provenance->tags.erase(a);
  } else {
    std::lock_guard<std::mutex> g{s_static_provenance_lock};
    s_static_array_provenance.erase(a);
  }
}


} // namespace

folly::Optional<Tag> getTag(const ArrayData* ad) {
  if (!ad->hasProvenanceData()) return folly::none;
  auto const tag = getTagImpl(ad);
  assertx(tag);
  return tag;
}
folly::Optional<Tag> getTag(const APCArray* a) {
  return getTagImpl(a);
}

template<Mode mode>
void setTag(ArrayData* ad, Tag tag) {
  if (setTagImpl<mode>(ad, tag)) {
    ad->setHasProvenanceData(true);
  }
}
template<Mode mode>
void setTag(const APCArray* a, Tag tag) {
  setTagImpl<mode>(a, tag);
}

template void setTag<Mode::Insert>(ArrayData*, Tag);
template void setTag<Mode::Emplace>(ArrayData*, Tag);
template void setTag<Mode::Insert>(const APCArray*, Tag);
template void setTag<Mode::Emplace>(const APCArray*, Tag);

void clearTag(ArrayData* ad) {
  ad->setHasProvenanceData(false);
  clearTagImpl(ad);
}
void clearTag(const APCArray* a) {
  clearTagImpl(a);
}

namespace {

void tagTVImpl(TypedValue& tv, folly::Optional<Tag> tag) {
  assertx(RuntimeOption::EvalArrayProvenance);

  if (!tvWantsTag(tv)) return;

  auto ad = val(tv).parr;
  if (ad->hasProvenanceData()) return;

  if (!tag) tag = tagFromPC();
  if (!tag) return;

  if (!ad->hasExactlyOneRef()) {
    ad = ad->copy();

    TypedValue tmp;
    type(tmp) = dt_with_rc(type(tv));
    val(tmp).parr = ad;

    tvMove(tmp, tv);
  }
  // the copy() above may have tagged this array with the PC data
  // so we can't assert that it's not there--this is safe since
  // we bail out above if the input array was already tagged
  setTag<Mode::Emplace>(ad, *tag);
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
  assertx(RuntimeOption::EvalArrayProvenance);
  assertx(base->empty());
  assertx(base->isStatic());
  assertx(arrayWantsTag(base));

  if (!tag) tag = tagFromPC();
  if (!tag) return base;

  auto ad = copy(base);
  assertx(ad);
  assertx(ad->hasExactlyOneRef());

  arrprov::setTag<Mode::Emplace>(ad, *tag);
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

folly::Optional<Tag> tagFromPC() {
  VMRegAnchor _(VMRegAnchor::Soft);
  if (tl_regState != VMRegState::CLEAN || vmfp() == nullptr) {
    return folly::none;
  }

  auto const tag = fromLeaf(
    [&] (const ActRec* fp, Offset offset) -> folly::Optional<Tag> {
      auto const func = fp->func();
      auto const unit = fp->unit();
      // grab the filename off the Func* since it might be different
      // from the unit's for flattened trait methods
      auto const filename = func->filename();
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
