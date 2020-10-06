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
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/util/stack-trace.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/type-scan.h"
#include "hphp/util/type-traits.h"

#include <folly/AtomicHashMap.h>
#include <folly/container/F14Map.h>
#include <folly/Format.h>
#include <folly/SharedMutex.h>
#include <tbb/concurrent_hash_map.h>

#include <type_traits>

namespace HPHP { namespace arrprov {

TRACE_SET_MOD(runtime);

////////////////////////////////////////////////////////////////////////////////

namespace {

using TagID = uint32_t;
using TagStorage = std::pair<LowPtr<const StringData>, int32_t>;

static constexpr TagID kKindBits = 3;
static constexpr TagID kKindMask = 0x7;
static constexpr size_t kMaxTagID = (1 << (8 * sizeof(TagID) - kKindBits)) - 1;

struct TagHashCompare {
  bool equal(TagStorage a, TagStorage b) const {
    return a == b;
  }
  size_t hash(TagStorage a) const {
    static_assert(IMPLIES(use_lowptr, sizeof(TagStorage) == sizeof(int64_t)));
    if constexpr (use_lowptr) {
      auto result = int64_t{};
      memcpy(&result, &a, sizeof(TagStorage));
      return hash_int64(result);
    }
    auto const first = safe_cast<int64_t>(uintptr_t(a.first.get()));
    return hash_int64_pair(first, a.second);
  }
};

using TagIDs = tbb::concurrent_hash_map<TagStorage, TagID, TagHashCompare>;

static std::atomic<size_t> s_numTags;
static TagIDs s_tagIDs;

TagStorage* getRawTagStorageArray() {
  assertx(!use_lowptr || RO::EvalArrayProvenance);
  static auto const result = reinterpret_cast<TagStorage*>(
    malloc((kMaxTagID + 1) * sizeof(TagStorage))
  );
  return result;
}

TagStorage getTagStorage(TagID i) {
  return getRawTagStorageArray()[i];
}

TagID getTagID(TagStorage tag) {
  {
    TagIDs::const_accessor it;
    if (s_tagIDs.find(it, tag)) return it->second;
  }
  TagIDs::accessor it;
  if (s_tagIDs.insert(it, tag)) {
    auto const i = s_numTags++;
    always_assert(i <= kMaxTagID);
    getRawTagStorageArray()[i] = tag;
    it->second = i;
  }
  return it->second;
}

}

////////////////////////////////////////////////////////////////////////////////

Tag::Tag(const Func* func, Offset offset) {
  // Builtins have empty filenames, so use the unit; else use func->filename
  // in order to resolve the original filenames of flattened traits.
  auto const unit = func->unit();
  auto const file = func->isBuiltin() ? unit->filepath() : func->filename();
  *this = Known(file, unit->getLineNumber(offset));
}

Tag::Tag(Tag::Kind kind, const StringData* name, int32_t line) {
  auto const k = TagID(kind);
  assertx(k < kKindMask);
  assertx((k & kKindMask) == k);
  assertx((k & uintptr_t(name)) == 0);
  assertx(kind != Kind::Invalid);

  if (kind == Kind::Known) {
    m_id = k | (getTagID({name, line}) << kKindBits);
  } else if (uintptr_t(name) <= std::numeric_limits<TagID>::max()) {
    m_id = k | safe_cast<TagID>(uintptr_t(name));
  } else {
    m_id = kKindMask | (getTagID({name, -int(k)}) << kKindBits);
  }

  // Check that we can undo tag compression and get back the original values.
  assertx(this->kind() == kind);
  assertx(this->name() == name);
  assertx(this->line() == line);
}

Tag::Kind Tag::kind() const {
  auto const bits = m_id & kKindMask;
  if (bits < kKindMask) return Tag::Kind(bits);
  return Tag::Kind(-getTagStorage(size_t(m_id) >> kKindBits).second);
}
const StringData* Tag::name() const {
  auto const bits = m_id & kKindMask;
  if (bits < TagID(Kind::Known)) {
    return reinterpret_cast<StringData*>(m_id & ~kKindMask);
  }
  return getTagStorage(size_t(m_id) >> kKindBits).first;
}
int32_t Tag::line() const {
  auto const bits = m_id & kKindMask;
  if (bits != TagID(Kind::Known)) return -1;
  return getTagStorage(size_t(m_id) >> kKindBits).second;
}
uint64_t Tag::hash() const {
  return m_id;
}

bool Tag::operator==(const Tag& other) const {
  return m_id == other.m_id;
}
bool Tag::operator!=(const Tag& other) const {
  return m_id != other.m_id;
}

std::string Tag::toString() const {
  switch (kind()) {
  case Kind::Invalid:
    return "unknown location (no tag)";
  case Kind::Known:
    return folly::sformat("{}:{}", name()->slice(), line());
  case Kind::UnknownRepo:
    return "unknown location (repo union)";
  case Kind::KnownTraitMerge:
    return folly::sformat("{}:{} (trait xinit merge)", name()->slice(), -1);
  case Kind::KnownLargeEnum:
    return folly::sformat("{}:{} (large enum)", name()->slice(), -1);
  case Kind::RuntimeLocation:
    return folly::sformat("{} (c++ runtime location)", name()->slice());
  case Kind::RuntimeLocationPoison:
    return folly::sformat("unknown location (poisoned c++ runtime location was {})",
                          name()->slice());
  }
  always_assert(false);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

RDS_LOCAL(Tag, rl_tag_override);
RDS_LOCAL(ArrayProvenanceTable, rl_array_provenance);

/*
 * Flush the table after each request since none of the ArrayData*s will be
 * valid anymore.
 */
InitFiniNode flushTable([]{
  if (!RO::EvalArrayProvenance) return;
  rl_array_provenance->tags.clear();
  assert_flog(!rl_tag_override->valid(),
              "contents: {}",
              rl_tag_override->toString());
}, InitFiniNode::When::RequestFini);

} // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

bool arrayWantsTag(const ArrayData* ad) {
  return RO::EvalArrayProvenance && ad->isDVArray() && !ad->isLegacyArray();
}

bool arrayWantsTag(const APCArray* a) {
  return RO::EvalArrayProvenance && (a->isVArray() || a->isDArray());
}

bool arrayWantsTag(const AsioExternalThreadEvent* ev) {
  return true;
}

/*
 * Mutable access to a given object's provenance slot.
 */
void Tag::set(ArrayData* ad, Tag tag) {
  assertx(ad->isVanilla());
  static_assert(sizeof(decltype(ad->m_extra)) == sizeof(Tag));
  ad->m_extra = folly::bit_cast<uint32_t>(tag);
}

void Tag::set(APCArray* a, Tag tag) {
  auto mem = reinterpret_cast<Tag*>(
    reinterpret_cast<char*>(a) - kAPCTagSize
  );
  *mem = tag;
}

void Tag::set(AsioExternalThreadEvent* a, Tag tag) {
  if (tag.valid()) {
    rl_array_provenance->tags[a] = tag;
  } else {
    rl_array_provenance->tags.erase(a);
  }
}

/*
 * Const access to a given object's provenance slot.
 */
Tag Tag::get(const ArrayData* ad) {
  assertx(ad->isVanilla());
  static_assert(sizeof(decltype(ad->m_extra)) == sizeof(Tag));
  return folly::bit_cast<Tag>(ad->m_extra);
}

Tag Tag::get(const APCArray* a) {
  auto const mem = reinterpret_cast<const Tag*>(
    reinterpret_cast<const char*>(a) - kAPCTagSize
  );
  return *mem;
}

Tag Tag::get(const AsioExternalThreadEvent* a) {
  auto const& table = rl_array_provenance->tags;
  auto const it = table.find(a);
  if (it == table.end()) return {};
  assertx(it->second.valid());
  return it->second;
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

} // namespace

Tag getTag(const ArrayData* ad) {
  assertx(RO::EvalArrayProvenance);
  if (tl_tag_override) return *tl_tag_override;
  return Tag::get(ad);
}

Tag getTag(const APCArray* a) {
  assertx(RO::EvalArrayProvenance);
  return Tag::get(a);
}

Tag getTag(const AsioExternalThreadEvent* ev) {
  assertx(RO::EvalArrayProvenance);
  return Tag::get(ev);
}

void setTag(ArrayData* ad, Tag tag) {
  assertx(RO::EvalArrayProvenance);
  assertx(tag.valid());
  if (!arrayWantsTag(ad)) return;
  Tag::set(ad, tag);
}

void setTag(APCArray* a, Tag tag) {
  assertx(RO::EvalArrayProvenance);
  assertx(tag.valid());
  if (!arrayWantsTag(a)) return;
  Tag::set(a, tag);
}

void setTag(AsioExternalThreadEvent* ev, Tag tag) {
  assertx(RO::EvalArrayProvenance);
  assertx(tag.valid());
  if (!arrayWantsTag(ev)) return;
  Tag::set(ev, tag);
}

void clearTag(ArrayData* ad) {
  assertx(RO::EvalArrayProvenance);
  Tag::set(ad, {});
}

void clearTag(APCArray* a) {
  assertx(RO::EvalArrayProvenance);
  Tag::set(a, {});
}

void clearTag(AsioExternalThreadEvent* ev) {
  assertx(RO::EvalArrayProvenance);
  Tag::set(ev, {});
}

void reassignTag(ArrayData* ad) {
  assertx(RO::EvalArrayProvenance);
  if (arrayWantsTag(ad)) {
    if (auto const tag = tagFromPC()) {
      setTag(ad, tag);
      return;
    }
  }
  clearTag(ad);
}

ArrayData* tagStaticArr(ArrayData* ad, Tag tag /* = {} */) {
  assertx(RO::EvalArrayProvenance);
  assertx(ad->isStatic());
  assertx(arrayWantsTag(ad));

  if (!tag.valid()) tag = tagFromPC();
  if (!tag.valid()) return ad;

  tl_tag_override = tag;
  SCOPE_EXIT { tl_tag_override = folly::none; };

  ArrayData::GetScalarArray(&ad, tag);
  return ad;
}

///////////////////////////////////////////////////////////////////////////////

TagOverride::TagOverride(Tag tag)
  : m_valid(RO::EvalArrayProvenance)
{
  if (m_valid) {
    m_saved_tag = *rl_tag_override;
    *rl_tag_override = tag;
  }
}

TagOverride::TagOverride(Tag tag, TagOverride::ForceTag)
  : m_valid(true)
{
  if (m_valid) {
    m_saved_tag = *rl_tag_override;
    *rl_tag_override = tag;
  }
}

TagOverride::~TagOverride() {
  if (m_valid) {
    *rl_tag_override = m_saved_tag;
  }
}

Tag tagFromPC() {
  if (!RO::EvalArrayProvenance) return {};

  auto log_violation = [&](const char* why) {
    auto const rate = RO::EvalLogArrayProvenanceDiagnosticsSampleRate;
    if (StructuredLog::coinflip(rate)) {
      StructuredLogEntry sle;
      sle.setStr("reason", why);
      sle.setStackTrace("stack", StackTrace{StackTrace::Force{}});
      FTRACE_MOD(Trace::runtime, 2, "arrprov {} {}\n", why, show(sle));
      StructuredLog::log("hphp_arrprov_diagnostics", sle);
    }
  };

  if (rl_tag_override->valid()) {
    if (rl_tag_override->kind() == Tag::Kind::RuntimeLocationPoison) {
      log_violation("poison");
    }
    return *rl_tag_override;
  }

  VMRegAnchor _(VMRegAnchor::Soft);

  if (tl_regState != VMRegState::CLEAN || vmfp() == nullptr) {
    log_violation("no_fixup");
    return {};
  }

  auto const make_tag = [&] (const ActRec* fp, Offset offset) {
    return Tag { fp->func(), offset };
  };

  auto const skip_frame = [] (const ActRec* fp) {
    return !fp->func()->isProvenanceSkipFrame() &&
           !fp->func()->isCPPBuiltin();
  };

  auto const tag = fromLeaf(make_tag, skip_frame);
  assertx(!tag.valid() || tag.concrete());
  return tag;
}

Tag tagFromSK(SrcKey sk) {
  assert(sk.valid());
  if (!RO::EvalArrayProvenance) return {};
  return Tag { sk.func(), sk.offset() };
}

///////////////////////////////////////////////////////////////////////////////

namespace {

static auto const kMaxMutationStackDepth = 512;

template <typename Mutation>
struct MutationState {
  Mutation& mutation;
  const char* function_name;
  bool recursive = true;
  bool raised_stack_notice = false;
};

template <typename State>
ArrayData* apply_mutation(TypedValue tv, State& state,
                          bool cow = false, uint32_t depth = 0);

template <typename Array>
tv_lval LvalAtIterPos(ArrayData* ad, ssize_t pos) {
  if constexpr (std::is_same<Array, PackedArray>::value) {
    return PackedArray::LvalUncheckedInt(ad, pos);
  } else {
    static_assert(std::is_same<Array, MixedArray>::value);
    return &MixedArray::asMixed(ad)->data()[pos].data;
  }
}

template <typename Array, typename State>
ArrayData* apply_mutation_fast(ArrayData* in, ArrayData* result,
                               State& state, bool cow, uint32_t depth) {
  auto const end = Array::IterEnd(in);
  for (auto pos = Array::IterBegin(in); pos != end;
       pos = Array::IterAdvance(in, pos)) {
    auto const prev = *LvalAtIterPos<Array>(in, pos);
    auto const ad = apply_mutation(prev, state, cow, depth + 1);
    if (!ad) continue;

    auto const next = make_array_like_tv(ad);
    result = result ? result : cow ? Array::Copy(in) : in;
    assertx(result->hasExactlyOneRef());
    assertx(Array::IterEnd(result) == Array::IterEnd(in));
    tvMove(next, LvalAtIterPos<Array>(result, pos));
  }
  return result == in ? nullptr : result;
}

template <typename State>
ArrayData* apply_mutation_slow(ArrayData* in, ArrayData* result,
                               State& state, bool cow, uint32_t depth) {
  IterateKVNoInc(in, [&](auto key, auto prev) {
    auto const ad = apply_mutation(prev, state, cow, depth + 1);
    if (!ad) return;

    auto const next = make_array_like_tv(ad);
    if (result || !cow) {
      result = result ? result : in;
      auto const escalated = result->set(key, next);
      assertx(escalated->hasExactlyOneRef());
      if (escalated == result) return;
      if (result != in) result->release();
      result = escalated;
    } else {
      in->incRefCount();
      SCOPE_EXIT { in->decRefCount(); };
      result = in->set(key, next);
      assertx(result->hasExactlyOneRef());
    }
  });
  return result == in ? nullptr : result;
}

// This function applies `state.mutation` to `tv` to get a modified array-like.
// Then, if `state.recursive` is set, it recursively applies the mutation to
// the values of the array-like. It does so with the minimum number of copies,
// mutating each array in-place if possible.
//
// `state.mutation` should take an ArrayData* and a `cow` param. If it can
// mutate the array in place (that is, either `cow` is false or no mutation is
// needed at all), it should do so and return nullptr. Otherwise, it must copy
// the array, mutate it, and return the updated result.
//
// We pass the mutation callback a `cow` param instead of checking ad->cowCheck
// to handle cases such as a refcount 1 array contained in a refcount 2 array;
// even though cowCheck return false for the refcount 1 array, we still need to
// copy it to get a new value to store in the COW-ed containing array.
//
// This method doesn't recurse into object properties; instead, if we encounter
// an object, we'll log (up to one) notice including `state.function_name`.
template <typename State>
ArrayData* apply_mutation(TypedValue tv, State& state,
                          bool cow, uint32_t depth) {
  if (depth == kMaxMutationStackDepth) {
    if (!state.raised_stack_notice) {
      raise_notice("%s stack depth exceeded!", state.function_name);
      state.raised_stack_notice = true;
    }
    return nullptr;
  }

  if (!isArrayLikeType(type(tv))) {
    return nullptr;
  }

  // Apply the mutation to the top-level array.
  auto const in = val(tv).parr;
  cow |= in->cowCheck();
  auto result = state.mutation(in, cow);
  if (!state.recursive) return result;

  // Recursively apply the mutation to the array's contents. For efficiency,
  // we do the layout check outside of the iteration loop.
  if (in->hasVanillaPackedLayout()) {
    return apply_mutation_fast<PackedArray>(in, result, state, cow, depth);
  } else if (in->hasVanillaMixedLayout()) {
    return apply_mutation_fast<MixedArray>(in, result, state, cow, depth);
  }
  return apply_mutation_slow(in, result, state, cow, depth);
}

TypedValue markTvImpl(TypedValue in, bool legacy, bool recursive) {
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
      if (ad->isVecType()) {
        warn_once(raised_hack_array_notice, Strings::ARRAY_MARK_LEGACY_VEC);
        return nullptr;
      } else if (ad->isDictType()) {
        warn_once(raised_hack_array_notice, Strings::ARRAY_MARK_LEGACY_DICT);
        return nullptr;
      } else if (ad->isNotDVArray()) {
        warn_once(raised_non_hack_array_notice,
                  "array_mark_legacy expects a varray or darray");
        return nullptr;
      }
    } else if (!ad->isVecType() && !ad->isDictType()) {
      warn_once(raised_non_hack_array_notice,
                "array_mark_legacy expects a dict or vec");
      return nullptr;
    }

    auto const result = ad->setLegacyArray(cow, true);
    return result == ad ? nullptr : result;
  };

  // Unmark legacy vecs/dicts to silence logging,
  // e.g. while casting to regular vecs or dicts.
  auto const unmark_tv = [&](ArrayData* ad, bool cow) -> ArrayData* {
    if (RO::EvalHackArrDVArrs && !ad->isVecType() && !ad->isDictType()) {
      return nullptr;
    }
    if (!RO::EvalHackArrDVArrs && !ad->isDVArray()) return nullptr;

    auto const result = ad->setLegacyArray(cow, false);
    return result == ad ? nullptr : result;
  };

  auto const ad = [&] {
    if (legacy) {
      auto state = MutationState<decltype(mark_tv)>{
        mark_tv, "array_mark_legacy", recursive};
      return apply_mutation(in, state);
    } else {
      auto state = MutationState<decltype(unmark_tv)>{
        unmark_tv, "array_unmark_legacy", recursive};
      return apply_mutation(in, state);
    }
  }();
  return ad ? make_array_like_tv(ad) : tvReturn(tvAsCVarRef(&in));
}

TypedValue tagTvImpl(TypedValue in, int64_t flags) {
  // Closure state: an expensive-to-compute provenance tag.
  folly::Optional<arrprov::Tag> tag = folly::none;

  // The closure: tag array-likes if they want tags, else leave them alone.
  auto const tag_tv = [&](ArrayData* ad, bool cow) -> ArrayData* {
    if (!arrprov::arrayWantsTag(ad)) return nullptr;
    if (!tag) tag = arrprov::tagFromPC();
    if (!tag->valid()) return nullptr;
    auto const result = [&]{
      if (!cow) return ad;
      auto const packed = ad->hasVanillaPackedLayout();
      return packed ? PackedArray::Copy(ad) : MixedArray::Copy(ad);
    }();
    arrprov::setTag(result, *tag);
    return cow ? result : nullptr;
  };

  auto state = MutationState<decltype(tag_tv)>{tag_tv, "tag_provenance_here"};
  auto const ad = apply_mutation(in, state);
  return ad ? make_array_like_tv(ad) : tvReturn(tvAsCVarRef(&in));
}

}

TypedValue tagTvRecursively(TypedValue in, int64_t flags) {
  if (!RO::EvalArrayProvenance) return tvReturn(tvAsCVarRef(&in));
  return tagTvImpl(in, flags);
}

TypedValue markTvRecursively(TypedValue in, bool legacy) {
  return markTvImpl(in, legacy, /*recursive=*/true);
}

TypedValue markTvShallow(TypedValue in, bool legacy) {
  return markTvImpl(in, legacy, /*recursive=*/false);
}

///////////////////////////////////////////////////////////////////////////////

}}
