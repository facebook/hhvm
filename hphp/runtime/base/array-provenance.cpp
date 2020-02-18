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

TRACE_SET_MOD(runtime);

////////////////////////////////////////////////////////////////////////////////

std::string Tag::toString() const {
  assertx(m_filename);
  return folly::sformat("{}:{}", m_filename->slice(), m_line);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

RDS_LOCAL_NO_CHECK(Tag, rl_tag_override);
RDS_LOCAL(ArrayProvenanceTable, rl_array_provenance);
folly::F14FastMap<const void*, Tag> s_static_array_provenance;
std::mutex s_static_provenance_lock;

/*
 * Flush the table after each request since none of the ArrayData*s will be
 * valid anymore
 */
InitFiniNode flushTable([]{
  if (!RO::EvalArrayProvenance) return;
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
constexpr bool wants_local_prov(const AsioExternalThreadEvent* ev) {
  return true;
}
constexpr bool wants_local_prov(const APCArray* a) { return false; }

}

bool arrayWantsTag(const ArrayData* ad) {
  return !ad->isLegacyArray() && (
    (RO::EvalArrProvHackArrays && (ad->isVecArray() || ad->isDict())) ||
    (RO::EvalArrProvDVArrays && (ad->isVArray() || ad->isDArray()))
  );
}

bool arrayWantsTag(const APCArray* a) {
  return (
    (RO::EvalArrProvHackArrays && (a->isVec() || a->isDict())) ||
    (RO::EvalArrProvDVArrays && (a->isVArray() || a->isDArray()))
  );
}

bool arrayWantsTag(const AsioExternalThreadEvent* ev) {
  return true;
}

namespace {

/*
 * Used to override the provenance tag reported for ArrayData*'s in a given
 * thread.
 *
 * This is pretty hacky, but it's only used for one specific purpose: for
 * obtaining a copy of a static array which has specific provenance.
 *
 * The static array cache is set up to distinguish arrays by provenance tag.
 * However, it's a tbb::concurrent_hash_set, which we can't jam a tag into.
 * Instead, its hash and equal functions look up the provenance tag of an array
 * in order to allow for multiple identical static arrays with different source
 * tags.
 *
 * As a result, there's no real way to thread a tag into the lookups and
 * inserts of the hash set.  We could pass in tagged temporary empty arrays,
 * but we don't want to keep allocating those.  We could keep one around for
 * each thread... but that's pretty much the moral equivalent of doing things
 * this way:
 *
 * So instead, we have a thread-local tag that is only "active" when we're
 * trying to retrieve or create a specifically tagged copy of a static array,
 * which facilitates the desired behavior in the static array cache.
 */
thread_local folly::Optional<Tag> tl_tag_override = folly::none;

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
  assertx(mode == Mode::Emplace || !getTag(a) || tl_tag_override);

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
  if (tl_tag_override) return tl_tag_override;
  if (!ad->hasProvenanceData()) return folly::none;
  auto const tag = getTagImpl(ad);
  assertx(tag);
  return tag;
}
folly::Optional<Tag> getTag(const APCArray* a) {
  return getTagImpl(a);
}
folly::Optional<Tag> getTag(const AsioExternalThreadEvent* ev) {
  return getTagImpl(ev);
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

template <Mode mode>
void setTag(AsioExternalThreadEvent* ev, Tag tag) {
  setTagImpl<mode>(ev, tag);
}

template void setTag<Mode::Insert>(ArrayData*, Tag);
template void setTag<Mode::Emplace>(ArrayData*, Tag);
template void setTag<Mode::Insert>(const APCArray*, Tag);
template void setTag<Mode::Emplace>(const APCArray*, Tag);
template void setTag<Mode::Insert>(AsioExternalThreadEvent*, Tag);
template void setTag<Mode::Emplace>(AsioExternalThreadEvent*, Tag);

void clearTag(ArrayData* ad) {
  ad->setHasProvenanceData(false);
  clearTagImpl(ad);
}
void clearTag(const APCArray* a) {
  clearTagImpl(a);
}
void clearTag(AsioExternalThreadEvent* ev) {
  clearTagImpl(ev);
}

void reassignTag(ArrayData* ad) {
  if (arrayWantsTag(ad)) {
    if (auto const tag = tagFromPC()) {
      setTag<Mode::Emplace>(ad, *tag);
      return;
    }
  }

  clearTag(ad);
}

ArrayData* tagStaticArr(ArrayData* ad, folly::Optional<Tag> tag) {
  assertx(RO::EvalArrayProvenance);
  assertx(ad->isStatic());
  assertx(arrayWantsTag(ad));

  if (!tag) tag = tagFromPC();
  if (!tag) return ad;

  tl_tag_override = tag;
  SCOPE_EXIT { tl_tag_override = folly::none; };

  ArrayData::GetScalarArray(&ad, tag);
  return ad;
}

///////////////////////////////////////////////////////////////////////////////

TagOverride::TagOverride(Tag tag)
  : m_saved_tag(rl_tag_override.getInited()
                ? folly::make_optional<Tag>(*rl_tag_override)
                : folly::none)
{
  rl_tag_override.emplace(tag);
}

TagOverride::~TagOverride() {
  if (m_saved_tag) {
    *rl_tag_override = *m_saved_tag;
  } else {
    rl_tag_override.nullOut();
  }
}

folly::Optional<Tag> tagFromPC() {
  if (rl_tag_override.getInited()) return *rl_tag_override;

  VMRegAnchor _(VMRegAnchor::Soft);

  if (tl_regState != VMRegState::CLEAN ||
      rds::header() == nullptr ||
      vmfp() == nullptr) {
    return folly::none;
  }

  auto const make_tag = [&] (
    const ActRec* fp,
    Offset offset
  ) -> folly::Optional<Tag> {
    auto const func = fp->func();
    auto const unit = fp->unit();
    // grab the filename off the Func* since it might be different
    // from the unit's for flattened trait methods
    auto const filename = func->filename();
    auto const line = unit->getLineNumber(offset);
    return Tag { filename, line };
  };

  auto const skip_frame = [] (const ActRec* fp) {
    return !fp->func()->isProvenanceSkipFrame() &&
           !fp->func()->isCPPBuiltin();
  };

  auto const tag = fromLeaf(make_tag, skip_frame);
  assertx(!tag || tag->filename() != nullptr);
  return tag;
}

///////////////////////////////////////////////////////////////////////////////

namespace {

template <typename Mutation>
struct MutationState {
  Mutation& mutation;
  const char* function_name;
  bool recursive = true;
  bool raised_object_notice = false;
};

// Returns a copy of the given array that the caller may mutate in place.
// ArrayIter positions in the original array are also valid for the new one.
ArrayData* copy_if_needed(ArrayData* in, bool cow) {
  TRACE(3, "%s %d-element rc %d %s array\n",
        cow ? "Copying" : "Reusing", safe_cast<int32_t>(in->size()),
        in->count(), ArrayData::kindToString(in->kind()));
  auto const result = cow ? in->copy() : in;
  assertx(result->hasExactlyOneRef());
  assertx(result->iter_end() == in->iter_end());
  return result;
}

// This function applies `state.mutation` to `tv` to get a modified array-like.
// Then, if `state.recursive` is set, it recursively applies the mutation to
// the values of the array-like. It does so with the minimum number of copies,
// mutating each array in-place if possible.
//
// `state.mutation` should take an ArrayData* and a `cow` param. If it doesn't
// need to mutate the array-like, it should return nullptr; otherwise, it must
// copy the array if needed (based on `cow`) and return the updated result.
//
// We pass the mutation callback a `cow` param instead of checking ad->cowCheck
// to handle cases such as a refcount 1 array contained in a refcount 2 array;
// even though cowCheck return false for the refcount 1 array, we still need to
// copy it to get a new value to store in the COW-ed containing array.
//
// This method doesn't recurse into object properties; instead, if we encounter
// an object, we'll log (up to one) notice including `state.function_name`.
template <typename State>
ArrayData* apply_mutation(TypedValue tv, State& state, bool cow = false) {
  if (isObjectType(type(tv)) && !state.raised_object_notice) {
    auto const cls = val(tv).pobj->getClassName().data();
    raise_notice("%s called on object: %s", state.function_name, cls);
    state.raised_object_notice = true;
    return nullptr;
  } else if (!isArrayLikeType(type(tv))) {
    return nullptr;
  }

  // Apply the mutation to the top-level array.
  auto const in = val(tv).parr;
  cow |= in->cowCheck();
  auto result = state.mutation(in, cow);
  if (result == in) result->incRefCount();
  if (!state.recursive) return result;

  // Recursively apply the mutation to the array's contents.
  //
  // We use a local iter (which doesn't inc-ref or dec-ref its base) to make
  // logic clearer here, but it isn't necessary, strictly speaking, since we
  // check the `cow` flag instead of in->cowCheck() in copy_if_needed.
  ArrayIter ai(in, ArrayIter::local);
  for (auto done = in->empty(); !done; done = ai.nextLocal(in)) {
    auto const ad = apply_mutation(ai.nvSecondLocal(in).tv(), state, cow);
    if (!ad) continue;
    result = result ? result : copy_if_needed(in, cow);
    if (result == in) result->incRefCount();
    tvMove(make_array_like_tv(ad), ai.nvSecondLocal(result).as_lval());
    while (!ai.nextLocal(result)) {
      auto const rval = ai.nvSecondLocal(result).as_lval();
      auto const ad = apply_mutation(rval.tv(), state, cow);
      if (ad) tvMove(make_array_like_tv(ad), rval.as_lval());
    }
    break;
  }
  return result;
}

TypedValue markTvImpl(TypedValue in, bool recursive) {
  // Closure state: whether or not we've raised notices for array-likes.
  auto raised_hack_array_notice = false;
  auto raised_non_hack_array_notice = false;
  auto warn_once = [](bool& warned, const char* message) {
    if (!warned) raise_warning("%s", message);
    warned = true;
  };

  // The closure: pre-HAM, tag dvarrays and notice on vec / dicts / PHP arrays;
  // post-HAM, tag vecs and dicts and notice on any other array-like inputs.
  auto const mark_tv = [&](ArrayData* ad, bool cow) -> ArrayData* {
    if (!RO::EvalHackArrDVArrs) {
      if (ad->isVecArray()) {
        warn_once(raised_hack_array_notice, Strings::ARRAY_MARK_LEGACY_VEC);
        return nullptr;
      } else if (ad->isDict()) {
        warn_once(raised_hack_array_notice, Strings::ARRAY_MARK_LEGACY_DICT);
        return nullptr;
      } else if (ad->isNotDVArray()) {
        warn_once(raised_non_hack_array_notice,
                  "array_mark_legacy expects a varray or darray");
        return nullptr;
      }
    } else if (!ad->isVecArray() && !ad->isDict()) {
      warn_once(raised_non_hack_array_notice,
                "array_mark_legacy expects a dict or vec");
      return nullptr;
    }

    if (!RO::EvalHackArrDVArrs) assertx(ad->isDVArray());
    if (RO::EvalHackArrDVArrs) assertx(ad->isVecArray() || ad->isDict());
    if (ad->isLegacyArray()) return ad;

    auto result = copy_if_needed(ad, cow);
    result->setLegacyArray(true);
    return result;
  };

  auto state = MutationState<decltype(mark_tv)>{
    mark_tv, "array_mark_legacy", recursive};
  auto const ad = apply_mutation(in, state);
  return ad ? make_array_like_tv(ad) : tvReturn(tvAsCVarRef(&in));
}

TypedValue tagTvImpl(TypedValue in) {
  // Closure state: an expensive-to-compute provenance tag.
  using ProvTag = folly::Optional<arrprov::Tag>;
  folly::Optional<ProvTag> tag = folly::none;

  // The closure: tag array-likes if they want tags, else leave them alone.
  auto const tag_tv = [&](ArrayData* ad, bool cow) -> ArrayData* {
    if (!arrprov::arrayWantsTag(ad)) return nullptr;
    if (!tag) tag = arrprov::tagFromPC();
    if (!*tag) return nullptr;
    auto result = copy_if_needed(ad, cow);
    arrprov::setTag<arrprov::Mode::Emplace>(result, **tag);
    return result;
  };

  auto state = MutationState<decltype(tag_tv)>{tag_tv, "tag_provenance_here"};
  auto const ad = apply_mutation(in, state);
  return ad ? make_array_like_tv(ad) : tvReturn(tvAsCVarRef(&in));
}

}

TypedValue tagTvRecursively(TypedValue in) {
  if (!RO::EvalArrayProvenance) return tvReturn(tvAsCVarRef(&in));
  return tagTvImpl(in);
}

TypedValue markTvRecursively(TypedValue in) {
  return markTvImpl(in, /*recursive=*/true);
}

TypedValue markTvShallow(TypedValue in) {
  return markTvImpl(in, /*recursive=*/false);
}

///////////////////////////////////////////////////////////////////////////////

}}
