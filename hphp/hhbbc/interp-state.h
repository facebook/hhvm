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
#pragma once

#include <vector>
#include <string>
#include <map>

#include <boost/variant.hpp>

#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/bc.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

struct ClassAnalysis;
struct FuncAnalysis;

struct ClsConstantWork;

//////////////////////////////////////////////////////////////////////

/*
 * State of an iterator in the program.
 *
 * We track iterator liveness precisely, so if an iterator is DeadIter, its
 * definitely dead and vice-versa. We only track "normal" iterators (non-weak,
 * non-mutable), so iterators not of those type are considered "dead".
 *
 * We use this state to check if the "local iterator" optimization is valid.
 * In this optimization, we leave the iterator base in a local slot instead of
 * inc-ref-ing it and storing it in the iter.
 *
 * Local iteration is only possible if the positions of the keys of the base
 * iterator are unchanged. We call an update that leaves these key positions
 * unchanged a "safe" update; the only one that we can account for is updating
 * the value of the current iteration key, as in:
 *
 *  foreach ($base as $key => $value) {
 *    $base[$key] = $value + 1;
 *  }
 *
 * Finally, we also track a flag that is set whenever we see *any* update to
 * the base (including "safe" updates). If we know the base is unchanged during
 * iteration, we can further optimize the iterator in HHIR.
 */
struct DeadIter {};
struct LiveIter {
  IterTypes types;
  // If the base came from a local, and all updates to it have been "safe",
  // this field will be the id of that local. Otherwise, it will be NoLocalId.
  LocalId baseLocal       = NoLocalId;
  // The local that is known to be equivalent to the current key in the iter.
  // Used to detect "safe" updates to the base.
  LocalId keyLocal        = NoLocalId;
  // The block id where this iterator was initialized. If there's more than one
  // such block, it will be NoBlockId.
  BlockId initBlock       = NoBlockId;
  // Set whenever we see any mutation, even "safe" ones that don't affect keys.
  bool baseUpdated        = false;
  // Set whenever the base of the iterator cannot be an iterator
  bool baseCannotBeObject = false;
};
using Iter = boost::variant<DeadIter, LiveIter>;

/*
 * Tag indicating what sort of thing contains the current member base.
 *
 * Note that if we're in an array-chain, the base location always reflects the
 * location of the array which started the array-chain.
 */
enum class BaseLoc {
  None,

  /*
   * Base is in a number of possible places after an Elem op.  It
   * cannot possibly be in an object property (although it certainly
   * may alias one). See miElem for details.
   *
   * This is only used if its unclear if its actually in an array. If it is
   * definitely in an array, then arrayChain will be non-empty and the base
   * location will reflect where the array is located.
   */
  Elem,

  /*
   * Base is in possible locations after a Prop op.  This means it
   * possibly lives in a property on an object, but possibly not
   * (e.g. it could be a null in tvScratch).  See miProp for
   * details.
   *
   * If it is definitely known to be a property in an object, the
   * locTy in the Base will be a subtype of TObj.
   */
  Prop,

  /*
   * Known to be a static property on an object. This is only
   * possible as an initial base.
   */
  StaticProp,

  /*
   * Known to be contained in the current frame as a local, as the
   * frame $this, by the evaluation stack, or inside $GLOBALS.  Only
   * possible as initial bases.
   */
  Local,
  This,
  Stack,
  Global,
};

/*
 * Information about the current member base's type and location.
 */
struct Base {
  explicit Base(Type type = TCell,
                BaseLoc loc = BaseLoc::None,
                Type locTy = TCell,
                SString locName = {},
                LocalId locLocal = NoLocalId,
                uint32_t locSlot = 0)
    : type{std::move(type)}
    , loc{loc}
    , locTy{std::move(locTy)}
    , locName{locName}
    , locLocal{locLocal}
    , locSlot{locSlot} {}

  Type type;
  BaseLoc loc;

  /*
   * We also need to track effects of intermediate dims on the type of the base.
   * So we have a type, name, and possibly associated local or stack slot for
   * the base's container.
   *
   * For StaticProp, locName is the name of the property if known, or nullptr,
   * and locTy is the type of the class containing the static property.
   *
   * For Prop, locName is the name of the property if it was known, and locTy
   * gives as much information about the object type it is in.  (If we actually
   * *know* it is in an object, locTy will be a subtype of TObj.)
   *
   * For Local, locName is the name of the local if known, or nullptr, and
   * locLocal is the LocalId corresponding to the local (or NoLocalId if not
   * known).
   *
   * For Stack, locSlot is the stack index of the corresponding stack slot.
   */
  Type locTy;
  SString locName;
  LocalId locLocal;
  uint32_t locSlot;
};

// An element on the eval stack
struct StackElem {
  static auto constexpr NoId = std::numeric_limits<uint32_t>::max();

  Type type;
  // A location which is known to have an equivalent value to this
  // stack value. This could be a valid LocalId, the special value
  // StackDupId to indicate that its equivalent to the stack element
  // below it, the special value StackThisId to indicate that its the
  // value of $this, or NoLocalId if it has no known equivalents.
  // Note that the location may not match the stack value wrt Uninit.
  LocalId equivLoc;
  uint32_t index;
  uint32_t id;
};

struct InterpStack {
private:
  template<typename S>
  struct Iterator {
    friend struct InterpStack;
    Iterator(S* owner, uint32_t idx) :
        owner(owner), idx(idx) {
      assertx(idx <= owner->size());
    }
    void operator++() {
      assertx(idx < owner->size());
      ++idx;
    }
    void operator--() {
      assertx(idx);
      --idx;
    }
    Iterator operator-(ssize_t off) {
      return Iterator(owner, idx - off);
    }
    Iterator operator+(ssize_t off) {
      return Iterator(owner, idx + off);
    }
    auto& operator*() const {
      assertx(idx < owner->index.size());
      return owner->elems[owner->index[idx]];
    }
    auto* operator->() const {
      return &operator*();
    }
    // very special helper for use with Add*ElemC. To prevent
    // quadratic time/space when appending to an array, we need to
    // ensure we're appending to an array with a single reference; but
    // popping from an InterpStack doesn't actually remove the
    // element. This allows us to drop the type's datatag. It means
    // that rewinding wouldn't see the original type; but Add*ElemC is
    // very carefully coded anyway.
    auto unspecialize() const {
      auto t = std::move((*this)->type);
      (*this)->type = loosen_values(t);
      return t;
    }
    StackElem* next_elem(ssize_t off) const {
      const size_t i = owner->index[idx] + off;
      if (i > owner->elems.size()) return nullptr;
      return &owner->elems[i];
    }
    template<typename T>
    bool operator==(const Iterator<T>& other) const {
      return owner == other.owner && idx == other.idx;
    }
    template<typename T>
    bool operator!=(const Iterator<T>& other) const {
      return !operator==(other);
    }
    template<typename T>
    const Iterator& operator=(const Iterator<T>& other) const {
      owner = other.owner;
      idx = other.idx;
      return *this;
    }
  private:
    S* owner;
    uint32_t idx;
  };
public:
  using iterator = Iterator<InterpStack>;
  using const_iterator = Iterator<const InterpStack>;
  auto begin() { return iterator(this, 0); }
  auto end() { return iterator(this, index.size()); }
  auto begin() const { return const_iterator(this, 0); }
  auto end() const { return const_iterator(this, index.size()); }
  auto& operator[](size_t idx) { return *iterator(this, idx); }
  auto& operator[](size_t idx) const { return *const_iterator(this, idx); }
  auto& back() { return *iterator(this, index.size() - 1); }
  auto& back() const { return *const_iterator(this, index.size() - 1); }
  void push_elem(const StackElem& elm) {
    assertx(elm.index == index.size());
    index.push_back(elems.size());
    elems.push_back(elm);
  }
  void push_elem(StackElem&& elm) {
    assertx(elm.index == index.size());
    index.push_back(elems.size());
    elems.push_back(std::move(elm));
  }
  void push_elem(const Type& t, LocalId equivLoc,
                 uint32_t id = StackElem::NoId) {
    uint32_t isize = index.size();
    push_elem({t, equivLoc, isize, id});
  }
  void push_elem(Type&& t, LocalId equivLoc, uint32_t id = StackElem::NoId) {
    uint32_t isize = index.size();
    push_elem({std::move(t), equivLoc, isize, id});
  }
  void pop_elem() {
    index.pop_back();
  }
  void erase(iterator i1, iterator i2) {
    assertx(i1.owner == i2.owner);
    assertx(i1.idx < i2.idx);
    i1.owner->index.erase(i1.owner->index.begin() + i1.idx,
                          i1.owner->index.begin() + i2.idx);
  }
  bool empty() const { return index.empty(); }
  size_t size() const { return index.size(); }
  void clear() {
    index.clear();
    elems.clear();
  }
  void compact() {
    uint32_t i = 0;
    for (auto& ix : index) {
      if (ix != i) {
        assertx(ix > i);
        std::swap(elems[i], elems[ix]);
        ix = i;
      }
      ++i;
    }
    elems.resize(i);
  }
  // rewind the stack to the state it was in before the last
  // instruction ran (which is known to have popped numPop items and
  // pushed numPush items).
  void rewind(int numPop, int numPush);
  void peek(int numPop, const StackElem** values, int numPush) const;
  void kill(int numPop, int numPush, uint32_t id);
  void insert_after(int numPop, int numPush, const Type* types,
                    uint32_t numInst, uint32_t id);
private:
  void refill(size_t elemIx, size_t indexLow, int numPop, int numPush);
  CompactVector<uint32_t> index;
  CompactVector<StackElem> elems;
};

/*
 * A program state at a position in a php::Block.
 *
 * The `initialized' flag indicates whether the state knows anything.  All
 * other fields are invalid if a state is not initialized, and notably, after
 * all analysis has run, any blocks that still don't have initialized input
 * states are not reachable.
 *
 * The `unreachable' flag means we've produced this state from analysis, but
 * the program cannot reach this program position.  This flag has two uses:
 *
 *    o It allows us to determine arbitrary mid-block positions where code
 *      becomes unreachable, and eliminate that code in optimize.cpp.
 *
 *    o We may still do abstract interpretation of the unreachable code, but
 *      this flag is used when merging states to allow the interpreter to
 *      analyze blocks that are unreachable without pessimizing states for
 *      reachable blocks that would've been their successors.
 *
 * TODO: having the interpreter visit blocks when they are unreachable still
 * potentially merges types into object properties that aren't possible at
 * runtime.
 *
 * We split off a base class from State as a convenience to enable the use
 * default copy construction and assignment.
 *
 */
struct StateBase {
  StateBase() {
    initialized = unreachable = false;
  };
  StateBase(const StateBase&) = default;
  StateBase(StateBase&&) = default;
  StateBase& operator=(const StateBase&) = default;
  StateBase& operator=(StateBase&&) = default;

  uint8_t initialized : 1;
  uint8_t unreachable : 1;

  LocalId thisLoc = NoLocalId;
  Type thisType;
  CompactVector<Type> locals;
  CompactVector<Iter> iters;

  /*
   * Mapping of a local to other locals which are known to have
   * equivalent values. This equivalence ignores Uninit; users should
   * compare types if they care.
   */
  CompactVector<LocalId> equivLocals;
};

struct State : StateBase {
  State() = default;
  State(const State&) = default;
  State(State&&) = default;

  enum class Compact {};
  State(const State& src, Compact) : StateBase(src) {
    for (auto const& elm : src.stack) {
      stack.push_elem(elm.type, elm.equivLoc);
    }
  }

  // delete assignment operator, so we have to explicitly choose what
  // we want to do from amongst the various copies.
  State& operator=(const State&) = delete;
  State& operator=(State&&) = delete;

  void copy_from(const State& src) {
    *static_cast<StateBase*>(this) = src;
    stack = src.stack;
  }

  void copy_from(State&& src) {
    *static_cast<StateBase*>(this) = std::move(src);
    stack = std::move(src.stack);
  }

  void copy_and_compact(const State& src) {
    *static_cast<StateBase*>(this) = src;
    stack.clear();
    for (auto const& elm : src.stack) {
      stack.push_elem(elm.type, elm.equivLoc);
    }
  }

  void swap(State& other) {
    std::swap(static_cast<StateBase&>(*this), static_cast<StateBase&>(other));
    std::swap(stack, other.stack);
  }

  InterpStack stack;
};

/*
 * Return a copy of a State without copying the evaluation stack, pushing
 * Throwable on the stack.
 */
State with_throwable_only(const IIndex& env, const State&);

//////////////////////////////////////////////////////////////////////

/*
 * Undo log for mutations to the interp state. If mutating state, this
 * records enough information to undo that mutation and restore the
 * state to the previous values.
 */
struct StateMutationUndo {
  // Marks an instruction boundary, along with the flags during
  // interp.
  struct Mark {
    bool wasPEI = true;
    bool unreachable = false;
    decltype(StepFlags::mayReadLocalSet) mayReadLocalSet;
  };
  // Push to the stack (undone by a pop)
  struct Push {};
  // Pop from the stack (undone by pushing the recorded type)
  struct Pop { Type t; };
  // Location modification (undone by changing the local slot to the
  // recorded type)
  struct Local { LocalId id; Type t; };
  // Stack modification (undone by changing the stack slot to the
  // recorded type).
  struct Stack { size_t idx; Type t; };
  using Events = boost::variant<Push, Pop, Local, Stack, Mark>;

  std::vector<Events> events;

  void onPush() { events.emplace_back(Push{}); }
  void onPop(Type old) { events.emplace_back(Pop{ std::move(old) }); }
  void onStackWrite(size_t idx, Type old) {
    events.emplace_back(Stack{ idx, std::move(old) });
  }
  void onLocalWrite(LocalId l, Type old) {
    events.emplace_back(Local{ l, std::move(old) });
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * PropertiesInfo packages the PropState for private instance and
 * static properties, which is cross-block information collected in
 * CollectedInfo.
 *
 * During analysis the ClassAnalysis* is available and the PropState is
 * retrieved from there. However during optimization the ClassAnalysis is
 * not available and the PropState has to be retrieved off the Class in
 * the Index. In that case cls is nullptr and the PropState fields are
 * populated.
 */
struct PropertiesInfo {
  PropertiesInfo(const IIndex&, Context, ClassAnalysis*);

  const PropStateElem* readPrivateProp(SString name) const;
  const PropStateElem* readPrivateStatic(SString name) const;

  void mergeInPrivateProp(const IIndex& index,
                          SString name,
                          const Type& t);
  void mergeInPrivateStatic(const IIndex& index,
                            SString name,
                            const Type& t,
                            bool ignoreConst,
                            bool mustBeReadOnly);

  void mergeInPrivateStaticPreAdjusted(SString name, const Type& t);

  void mergeInAllPrivateProps(const IIndex&, const Type&);
  void mergeInAllPrivateStatics(const IIndex&, const Type&,
                                bool ignoreConst,
                                bool mustBeReadOnly);

  void setInitialValue(const php::Prop&, TypedValue, bool, bool);

  bool hasInitialValues() const { return !m_inits.empty(); }
  const PropInitInfo* getInitialValue(const php::Prop&) const;

  const PropState& privatePropertiesRaw() const;
  const PropState& privateStaticsRaw() const;

private:
  ClassAnalysis* const m_cls;
  PropState m_privateProperties;
  PropState m_privateStatics;
  hphp_fast_map<const php::Prop*, PropInitInfo> m_inits;
  const php::Func* m_func;
};

//////////////////////////////////////////////////////////////////////

/*
 * Encapsulates information about the current class's methods during
 * class-at-a-time analysis. This might be more refined than the
 * information in the Index.
 */
struct MethodsInfo {
  MethodsInfo(Context, ClassAnalysis*);

  // Look up the best known return type for the current class's
  // method, return std::nullopt if not known, or if the Func is not a
  // method of the current class.
  Optional<Index::ReturnType> lookupReturnType(const php::Func&);

private:
  ClassAnalysis* m_cls;
  const php::Func* m_func;
};

//////////////////////////////////////////////////////////////////////

/*
 * Map from closure classes to types for each of their used vars.
 * Shows up in a few different interpreter structures.
 */
using ClosureUseVarMap = hphp_fast_map<
  const php::Class*,
  CompactVector<Type>
>;

/*
 * Merge the types in the vector as possible use vars for the closure
 * `clo' into the destination map.
 */
void merge_closure_use_vars_into(ClosureUseVarMap& dst,
                                 const php::Class& clo,
                                 CompactVector<Type>);

//////////////////////////////////////////////////////////////////////

enum class CollectionOpts {
  Speculating = 1,
  Inlining = 2,
  EffectFreeOnly = 4,
  Optimizing = 8,
};

inline CollectionOpts operator|(CollectionOpts o1, CollectionOpts o2) {
  return static_cast<CollectionOpts>(
    static_cast<int>(o1) | static_cast<int>(o2)
  );
}

inline CollectionOpts operator&(CollectionOpts o1, CollectionOpts o2) {
  return static_cast<CollectionOpts>(
    static_cast<int>(o1) & static_cast<int>(o2)
  );
}

inline CollectionOpts operator-(CollectionOpts o1, CollectionOpts o2) {
  return static_cast<CollectionOpts>(
    static_cast<int>(o1) & ~static_cast<int>(o2)
  );
}

inline bool any(CollectionOpts o) { return static_cast<int>(o); }

/*
 * Area used for writing down any information that is collected across
 * a series of step operations (possibly cross block).
 */
struct CollectedInfo {
  CollectedInfo(const IIndex& index,
                Context ctx,
                ClassAnalysis* cls,
                CollectionOpts opts,
                ClsConstantWork* clsCns = nullptr,
                const FuncAnalysis* fa = nullptr);

  ClosureUseVarMap closureUseTypes;
  PropertiesInfo props;
  MethodsInfo methods;
  ClsConstantWork* clsCns;
  hphp_fast_set<CallContext, CallContextHasher> unfoldableFuncs;
  bool effectFree{true};
  bool hasInvariantIterBase{false};
  CollectionOpts opts{};
  /*
   * See FuncAnalysisResult for details.
   */
  std::bitset<64> usedParams;

  PublicSPropMutations publicSPropMutations;

  struct MInstrState {
    /*
     * The current member base. Updated as we move through bytecodes
     * representing the operation.
     */
    Base base{};

    /*
     * Used to track whether a member op sequence is effect free. We use
     * this information to replace member op sequences with constants.
     */
    bool effectFree{false};
    bool extraPop{false};

    /*
     * Chains of member operations on array elements will affect the type of
     * something further back in the member instruction. This vector tracks the
     * base,key type pair that was used at each stage. See
     * interp-minstr.cpp:resolveArrayChain().
     */
    struct ArrayChainEnt {
      Type base;
      Type key;
      LocalId keyLoc;
    };
    using ArrayChain = CompactVector<ArrayChainEnt>;
    ArrayChain arrayChain;

    void clear() {
      base.loc = BaseLoc::None;
      arrayChain.clear();
    }
  };
  MInstrState mInstrState;
};

//////////////////////////////////////////////////////////////////////

/*
 * State merging functions, based on the union_of operation for types.
 *
 * These return true if the destination state changed.
 */
bool merge_into(State&, const State&);

/*
 * State merging functions, based on the widening_union operation.
 * See analyze.cpp for details on when this is needed.
 */
bool widen_into(State&, const State&);

//////////////////////////////////////////////////////////////////////

/*
 * Functions to show various aspects of interpreter state as strings.
 */
std::string show(const php::Func&, const Base& b);
std::string show(const php::Func&, const CollectedInfo::MInstrState&);
std::string show(const php::Func&, const Iter&);
std::string property_state_string(const PropertiesInfo&);
std::string state_string(const php::Func&, const State&, const CollectedInfo&);

//////////////////////////////////////////////////////////////////////

}
