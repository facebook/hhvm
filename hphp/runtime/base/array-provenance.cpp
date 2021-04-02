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
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/req-hash-set.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/util/stack-trace.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/type-scan.h"
#include "hphp/util/type-traits.h"

#include <folly/AtomicHashMap.h>
#include <folly/Format.h>
#include <folly/SharedMutex.h>
#include <tbb/concurrent_hash_map.h>

#include <sys/mman.h>
#include <type_traits>

namespace HPHP { namespace arrprov {

TRACE_SET_MOD(runtime);

////////////////////////////////////////////////////////////////////////////////

namespace {

using TagID = uint32_t;
using TagStorage = std::pair<LowPtr<const StringData>, int32_t>;

static constexpr TagID kKindBits = 3;
static constexpr TagID kKindMask = 0x7;
static constexpr TagID kExternalStorageSentinel = 0x0;
static constexpr size_t kNumTagIDs = (1 << (8 * sizeof(TagID) - kKindBits));
static constexpr size_t kNumRuntimeTagIDs = (1 << 10);

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

struct TagMap {
  TagMap(size_t capacity) : capacity(capacity) {}

  TagStorage getTag(TagID i) {
    return getRawTagStorageArray()[i];
  }

  TagID getTagID(TagStorage tag) {
    {
      TagIDs::const_accessor it;
      if (tagIDs.find(it, tag)) return it->second;
    }
    TagIDs::accessor it;
    if (tagIDs.insert(it, tag)) {
      auto const i = numTags++;
      always_assert(i < capacity);
      getRawTagStorageArray()[i] = tag;
      it->second = i;
    }
    return it->second;
  }

private:
  using TagIDs = tbb::concurrent_hash_map<TagStorage, TagID, TagHashCompare>;

  TagStorage* getRawTagStorageArray() {
    if (allocation == nullptr) {
      auto const bytes = capacity * sizeof(TagStorage);
      auto const alloc = mmap(nullptr, bytes, PROT_READ|PROT_WRITE,
                              MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
      always_assert(alloc != MAP_FAILED);
      allocation = reinterpret_cast<TagStorage*>(alloc);
    }
    return allocation;
  }

  size_t capacity;
  TagStorage* allocation = nullptr;
  std::atomic<size_t> numTags = 0;
  TagIDs tagIDs;
};

static TagMap s_defaultTags(kNumTagIDs);
static TagMap s_runtimeTags(kNumRuntimeTagIDs);

// "indirect" kinds are kinds that are always stored in the TagStorage table.
// These are the only kinds that have both a name and a line number, because
// arrprov::Tag is only 4 bytes and cannot store both itself.
constexpr bool isIndirectKind(Tag::Kind kind) {
  return kind == Tag::Kind::Known || kind == Tag::Kind::KnownFuncParam;
}

// "runtime" kinds are associated with a location in the HHVM runtime, not the
// Hack code that it's executing. These kinds are special because we'll use
// them even if ArrayProvenance is disabled - in fact, we need to associate
// config-parsing arrays with such tags before we know the status of arrprov.
//
// The total number of runtime tag is tightly-bounded, as we'll only create
// these tags when we use ARRPROV_USE_RUNTIME_LOCATION or related macros in
// our C++ code. Right now, there are <64 of these tags.
constexpr bool isRuntimeKind(Tag::Kind kind) {
  return kind == Tag::Kind::RuntimeLocation ||
         kind == Tag::Kind::RuntimeLocationPoison;
}

const StringData* getFilename(const Func* func, const Unit* unit) {
  return func->isBuiltin() ? unit->filepath() : func->filename();
}

}

////////////////////////////////////////////////////////////////////////////////

Tag::Tag(const Func* func, Offset offset) {
  // Builtins have empty filenames, so use the unit; else use func->filename
  // in order to resolve the original filenames of flattened traits.
  auto const unit = func->unit();
  *this = Known(getFilename(func, unit), func->getLineNumber(offset));
}

Tag Tag::Param(const Func* func, int32_t param) {
  // The text computation here is expensive, but Param() is only used in the
  // interpreter and the JIT - the tag is burned into JIT-ed code for speed.
  assertx(func->fullName());
  auto const unit = func->unit();
  auto const file = getFilename(func, unit);
  auto const line = func->getLineNumber(0);
  auto const text = folly::to<std::string>(
      "param ", param, " of ", func->fullName()->data(), " at ", file->data());
  return Tag::Param(makeStaticString(text), line);
}

Tag::Tag(Tag::Kind kind, const StringData* name, int32_t line) {
  auto const k = TagID(kind);
  assertx((k & kKindMask) == k);
  assertx((k & uintptr_t(name)) == 0);
  assertx(k != kExternalStorageSentinel);
  assertx(kind != Kind::Invalid);

  // Assert that the default-constructed tag matches the default Array extra.
  static_assert(Tag().m_id == ArrayData::kDefaultVanillaArrayExtra);
  assertx(Tag().kind() == Kind::Invalid);

  if (isIndirectKind(kind)) {
    m_id = k | (s_defaultTags.getTagID({name, line}) << kKindBits);
  } else if (!use_lowptr && isRuntimeKind(kind)) {
    m_id = k | (s_runtimeTags.getTagID({name, line}) << kKindBits);
  } else if (uintptr_t(name) <= std::numeric_limits<TagID>::max()) {
    m_id = k | safe_cast<TagID>(uintptr_t(name));
  } else {
    m_id = s_defaultTags.getTagID({name, -int(k)}) << kKindBits;
  }

  // Check that we can undo tag compression and get back the original values.
  assertx(this->kind() == kind);
  assertx(this->name() == name);
  assertx(this->line() == line);
}

Tag::Kind Tag::kind() const {
  auto const bits = m_id & kKindMask;
  if (bits != kExternalStorageSentinel) return Tag::Kind(bits);
  return Tag::Kind(-s_defaultTags.getTag(size_t(m_id) >> kKindBits).second);
}
const StringData* Tag::name() const {
  auto const bits = m_id & kKindMask;
  if (bits == kExternalStorageSentinel || isIndirectKind(Kind(bits))) {
    return s_defaultTags.getTag(size_t(m_id) >> kKindBits).first;
  } else if (!use_lowptr && isRuntimeKind(Kind(bits))) {
    return s_runtimeTags.getTag(size_t(m_id) >> kKindBits).first;
  }
  return reinterpret_cast<StringData*>(m_id & ~kKindMask);
}
int32_t Tag::line() const {
  auto const bits = m_id & kKindMask;
  if (isIndirectKind(Kind(bits))) {
    return s_defaultTags.getTag(size_t(m_id) >> kKindBits).second;
  }
  return -1;
}

std::string Tag::toString() const {
  switch (kind()) {
  case Kind::Invalid:
    return "unknown location (no tag)";
  case Kind::Known:
  case Kind::KnownFuncParam:
    return folly::sformat("{}:{}", name()->slice(), line());
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
      FTRACE(2, "arrprov {} {}\n", why, show(sle));
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

using IgnoreCollections = bool;
using MutateCollections = req::fast_map<HeapObject*, ArrayData*>;

// NOTE: Setting a max_depth of 0 means that there's no user-provided limit.
// (We'll still stop at kMaxMutationStackDepth above for performance reasons.)
template <typename Mutation, typename Collections = IgnoreCollections>
struct MutationState {
  Mutation& mutation;
  const char* function_name;
  uint32_t max_depth = 0;
  bool raised_stack_notice = false;
  Collections visited{};
};

template <typename State>
constexpr bool shouldMutateCollections() {
  auto constexpr IsIgnoreCollections =
    std::is_same<decltype(State::visited), IgnoreCollections>::value;
  auto constexpr IsMutateCollections =
    std::is_same<decltype(State::visited), MutateCollections>::value;

  static_assert(IsIgnoreCollections || IsMutateCollections);

  return IsMutateCollections;
}

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
    assertx(result->hasExactlyOneRef() || shouldMutateCollections<State>());
    assertx(Array::IterEnd(result) == Array::IterEnd(in));
    tvMove(next, LvalAtIterPos<Array>(result, pos));
  }
  FTRACE(1, "Depth {}: {} {}\n", depth,
         result && result != in ? "copy" : "reuse", in);
  return result == in ? nullptr : result;
}

template <typename State>
ArrayData* apply_mutation_slow(ArrayData* in, ArrayData* result,
                               State& state, bool cow, uint32_t depth) {
  // Careful! Even IterateKV will take a refcount on unknown arrays.
  // In order to do the mutation in place when possible, we iterate by hand.
  auto const end = in->iter_end();
  for (auto pos = in->iter_begin(); pos != end; pos = in->iter_advance(pos)) {
    auto const prev = in->nvGetVal(pos);
    auto const ad = apply_mutation(prev, state, cow, depth + 1);
    if (!ad) continue;

    // TODO(kshaunak): We can avoid the copy here if !cow by modifying all
    // of these mutation helpers to have "setMove" semantics. But it doesn't
    // affect algorithmic complexity, since we already do O(n) iteration.
    auto const escalated = result ? result : in;
    if (escalated == in) in->incRefCount();

    auto const key = in->nvGetKey(pos);
    auto const next = make_array_like_tv(ad);
    result = escalated->setMove(key, next);
    assertx(result->hasExactlyOneRef());
  }
  FTRACE(1, "Depth {}: {} {}\n", depth,
         result && result != in ? "copy" : "reuse", in);
  return result == in ? nullptr : result;
}

template <typename State>
ArrayData* apply_mutation_to_array(ArrayData* in, State& state,
                                   bool cow, uint32_t depth) {
  FTRACE(1, "Depth {}: mutating {} (cow = {})\n", depth, in, cow);

  // Apply the mutation to the top-level array.
  cow |= in->cowCheck();
  auto result = state.mutation(in, cow);
  if (state.max_depth == depth + 1) {
    FTRACE(1, "Depth {}: {} {}\n", depth, result ? "copy" : "reuse", in);
    return result;
  }

  // Recursively apply the mutation to the array's contents. For efficiency,
  // we do the layout check outside of the iteration loop.
  if (in->isVanillaVec()) {
    return apply_mutation_fast<PackedArray>(in, result, state, cow, depth);
  } else if (in->isVanillaDict()) {
    return apply_mutation_fast<MixedArray>(in, result, state, cow, depth);
  }
  return apply_mutation_slow(in, result, state, cow, depth);
}

template <typename State>
ArrayData* apply_mutation_ignore_collections(TypedValue tv, State& state,
                                             bool cow, uint32_t depth) {
  if (!tvIsArrayLike(tv) || tvIsKeyset(tv)) {
    return nullptr;
  }
  assertx(tvIsVec(tv) || tvIsDict(tv));
  auto const arr = val(tv).parr;
  return apply_mutation_to_array(arr, state, cow, depth);
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
  return apply_mutation_ignore_collections(tv, state, cow, depth);
}

TypedValue markTvImpl(TypedValue in, bool legacy, uint32_t depth) {
  // Closure state: whether or not we've raised notices for array-likes.
  auto raised_non_hack_array_notice = false;
  auto warn_once = [](bool& warned, const char* message) {
    if (!warned) raise_warning("%s", message);
    warned = true;
  };

  // The closure: pre-HAM, tag dvarrays and notice on vec / dicts / PHP arrays;
  // post-HAM, tag vecs and dicts and notice on any other array-like inputs.
  auto const mark_tv = [&](ArrayData* ad, bool cow) -> ArrayData* {
    if (!ad->isVecType() && !ad->isDictType()) {
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
    if (!ad->isVecType() && !ad->isDictType()) {
      return nullptr;
    }
    auto const result = ad->setLegacyArray(cow, false);
    return result == ad ? nullptr : result;
  };

  auto const ad = [&] {
    if (legacy) {
      auto state = MutationState<decltype(mark_tv)>{
        mark_tv, "array_mark_legacy", depth};
      return apply_mutation(in, state);
    } else {
      auto state = MutationState<decltype(unmark_tv)>{
        unmark_tv, "array_unmark_legacy", depth};
      return apply_mutation(in, state);
    }
  }();
  return ad ? make_array_like_tv(ad) : tvReturn(tvAsCVarRef(&in));
}

}

TypedValue tagTvRecursively(TypedValue in, int64_t flags) {
  return tvReturn(tvAsCVarRef(&in));
}

TypedValue markTvRecursively(TypedValue in, bool legacy) {
  return markTvImpl(in, legacy, 0);
}

TypedValue markTvShallow(TypedValue in, bool legacy) {
  return markTvImpl(in, legacy, 1);
}

TypedValue markTvToDepth(TypedValue in, bool legacy, uint32_t depth) {
  return markTvImpl(in, legacy, depth);
}

///////////////////////////////////////////////////////////////////////////////

}}
