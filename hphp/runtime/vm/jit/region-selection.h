/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_JIT_REGION_SELECTION_H_
#define incl_HPHP_JIT_REGION_SELECTION_H_

#include <memory>
#include <utility>
#include <vector>
#include <string>

#include <boost/container/flat_map.hpp>

#include <folly/Format.h>
#include <folly/Optional.h>

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP { namespace jit {

struct MCGenerator;
struct ProfData;
struct TransCFG;

//////////////////////////////////////////////////////////////////////

enum class PGORegionMode {
  Hottrace, // Select a long region, using profile counters to guide the trace
  Hotblock, // Select a single block
  HotCFG,   // Select arbitrary CFG using profile counters to prune cold paths
  WholeCFG, // Select the entire CFG that has been profiled
};

//////////////////////////////////////////////////////////////////////

/*
 * RegionDesc is a description of a code region.  This includes the
 * list of blocks in the region, and also the list of control-flow
 * arcs within the region.
 *
 * It consists of a list of unique SrcKey ranges, with type
 * annotations that may come from profiling or other sources.
 *
 * The first block is the entry point, and the remaining blocks must
 * be sorted in a reverse post order.
 */
struct RegionDesc {
  struct Block;
  struct Arc;
  struct Location;
  struct TypedLocation;
  struct ReffinessPred;
  using BlockPtr = std::shared_ptr<Block>;
  using BlockId = TransID;
  // BlockId Encoding:
  //   - Non-negative numbers are blocks that correspond
  //     to the start of a TransProfile translation, and therefore can
  //     be used to index into ProfData.
  //   - Negative numbers are used for other blocks, which correspond
  //     to blocks created by inlining and which don't correspond to
  //     the beginning of a profiling translation.
  using BlockIdSet = boost::container::flat_set<BlockId>;
  using BlockIdVec = std::vector<BlockId>;
  using BlockVec = std::vector<BlockPtr>;

  bool              empty() const;
  SrcKey            start() const;
  BlockPtr          entry() const;
  const BlockVec&   blocks() const;
  BlockPtr          block(BlockId id) const;
  const BlockIdSet& succs(BlockId bid) const;
  const BlockIdSet& preds(BlockId bid) const;
  const BlockIdSet& sideExitingBlocks() const;
  bool              isExit(BlockId bid) const;

  /*
   * Modify this RegionDesc so that its list of blocks is sorted in a reverse
   * post order.
   */
  void sortBlocks();

  /*
   * Returns the last BC offset in the region that corresponds to the
   * function where the region starts.  This will normally be the offset
   * of the last instruction in the last block, except if the function
   * ends with an inlined call.  In this case, the offset of the
   * corresponding FCall* in the function that starts the region is
   * returned.
   *
   * Note that the notion of "last BC offset" only makes sense for
   * regions that are linear traces.
   */
  SrcKey            lastSrcKey() const;

  Block*            addBlock(SrcKey sk, int length, FPInvOffset spOffset,
                             uint16_t inlineLevel);
  void              deleteBlock(BlockId bid);
  void              renumberBlock(BlockId oldId, BlockId newId);
  void              addArc(BlockId src, BlockId dst);
  void              removeArc(BlockId src, BlockId dst);
  void              setSideExitingBlock(BlockId bid);
  bool              isSideExitingBlock(BlockId bid) const;
  folly::Optional<BlockId> nextRetrans(BlockId id) const;
  void              setNextRetrans(BlockId id, BlockId next);
  void              append(const RegionDesc&  other);
  void              prepend(const RegionDesc& other);
  void              chainRetransBlocks();
  uint32_t          instrSize() const;
  std::string       toString() const;

  template<class Work>
  void              forEachArc(Work w) const;

private:
  struct BlockData {
    BlockPtr                 block;
    BlockIdSet               preds;
    BlockIdSet               succs;
    folly::Optional<BlockId> nextRetrans;
    explicit BlockData(BlockPtr b = nullptr) : block(b) {}
  };

  bool       hasBlock(BlockId id) const;
  BlockData& data(BlockId id);
  void       copyBlocksFrom(const RegionDesc& other,
                            BlockVec::iterator where);
  void       copyArcsFrom(const RegionDesc& other);
  void       postOrderSort(RegionDesc::BlockId     bid,
                           RegionDesc::BlockIdSet& visited,
                           RegionDesc::BlockIdVec& outVec);

  std::vector<BlockPtr>             m_blocks;
  hphp_hash_map<BlockId, BlockData> m_data;
  // Set of blocks that that can possibly side exit the region. This
  // is just a hint to the region translator.
  BlockIdSet                        m_sideExitingBlocks;
};

using RegionDescPtr = std::shared_ptr<RegionDesc>;
using RegionVec = std::vector<RegionDescPtr>;
using RegionSet = hphp_hash_set<
  RegionDescPtr,
  smart_pointer_hash<RegionDescPtr>
>;

/*
 * Specification of an HHBC-visible location that can have a type
 * hint.  This is currently either local variables or stack slots.
 *
 * Local variables are addressed by local id, and stack slots are
 * addressed by offset from the top of the stack at the HHBC opcode
 * being annoted.  So Stack{0} is the top of the stack, Stack{1} is
 * one slot under the top, etc.
 */
struct RegionDesc::Location {
  enum class Tag : uint32_t {
    Local,
    Stack,
  };
  struct Local { uint32_t locId;  };
  struct Stack {
    FPInvOffset offsetFromFP;
  };

  /* implicit */ Location(Local l) : m_tag{Tag::Local}, m_local(l) {}
  /* implicit */ Location(Stack s) : m_tag{Tag::Stack}, m_stack(s) {}

  Tag tag() const { return m_tag; };

  uint32_t localId() const {
    assertx(m_tag == Tag::Local);
    return m_local.locId;
  }

  FPInvOffset offsetFromFP() const {
    assertx(m_tag == Tag::Stack);
    return m_stack.offsetFromFP;
  }

  bool operator==(const Location& other) const {
    if (m_tag != other.m_tag) return false;

    switch (m_tag) {
    case Tag::Local:
      return localId() == other.localId();
    case Tag::Stack:
      return offsetFromFP() == other.offsetFromFP();
    }
    not_reached();
    return false;
  }

  bool operator!=(const Location& other) const {
    return !(*this == other);
  }

  bool operator<(const Location& other) const {
    if (m_tag < other.m_tag) return true;
    if (m_tag > other.m_tag) return false;
    switch (m_tag) {
    case Tag::Local:
      return localId() < other.localId();
    case Tag::Stack:
      return offsetFromFP() < other.offsetFromFP();
    }
    not_reached();
    return false;
  }

private:
  Tag m_tag;
  union {
    Local m_local;
    Stack m_stack;
  };
};

struct RegionDesc::Arc {
  BlockId src;
  BlockId dst;
};

/*
 * A type for somewhere in the middle of or start of a region.
 *
 * All types annotated in the RegionDesc are things that need to be guarded on
 * or checked and then side-exited on. Types from ahead-of-time static analysis
 * are encoded in the bytecode stream.
 */
struct RegionDesc::TypedLocation {
  Location location;
  Type type;
};

inline bool operator==(const RegionDesc::TypedLocation& a,
                       const RegionDesc::TypedLocation& b) {
  return a.location == b.location && a.type == b.type;
}

/*
 * PostConditions are known type information for locals and stack
 * locations at the end of profiling translation.
 *
 * These are kept for blocks that end a profiling translation in order
 * to enable better region selection.  This information is used to
 * prevent profiling translations with incompatible types from being
 * stitched together in a larger, optimizing translation.
 *
 * The PostConditions are kept in two distinct sets:
 *
 *   - the 'changed' set includes locations that may have been
 *     modified by the corresponding translation;
 *
 *   - the 'refined' set includes locations that are not modified but
 *     that some information is learned about them during the
 *     corresponding translation, typically due to type checks or
 *     asserts.
 */
using TypedLocations = std::vector<RegionDesc::TypedLocation>;

struct PostConditions {
  TypedLocations changed;
  TypedLocations refined;
};

/*
 * A prediction for the argument reffiness of the Func for a pre-live ActRec.
 *
 * mask is a bitmask of all 1's, with one bit for each parameter being passed.
 *
 * vals is a bitmask of the same length as mask, with a 1 representing a
 * parameter that will be passed by reference and a 0 for for value.
 *
 * arSpOffset is the offset from the initialSpOffset to the ActRec.
 */
struct RegionDesc::ReffinessPred {
  std::vector<bool> mask;
  std::vector<bool> vals;
  int64_t arSpOffset;
};

inline bool operator==(const RegionDesc::ReffinessPred& a,
                       const RegionDesc::ReffinessPred& b) {
  return a.mask == b.mask && a.vals == b.vals && a.arSpOffset == b.arSpOffset;
}

/*
 * A basic block in the region, with type predictions for conditions
 * at various execution points, including at entry to the block.
 */
struct RegionDesc::Block {
  using TypedLocMap = boost::container::flat_multimap<SrcKey, TypedLocation>;
  using ParamByRefMap = boost::container::flat_map<SrcKey,bool>;
  using RefPredMap = boost::container::flat_multimap<SrcKey,ReffinessPred>;
  using KnownFuncMap = boost::container::flat_map<SrcKey,const Func*>;

  explicit Block(const Func* func, bool resumed, Offset start, int length,
                 FPInvOffset initSpOff, uint16_t inlineLevel);

  Block& operator=(const Block&) = delete;

  /*
   * Accessors for the func, unit, length (in HHBC instructions), and
   * starting SrcKey of this Block.
   */
  BlockId     id()                const { return m_id; }
  const Unit* unit()              const { return m_func->unit(); }
  const Func* func()              const { return m_func; }
  SrcKey      start()             const { return SrcKey { m_func, m_start,
                                                          m_resumed }; }
  SrcKey      last()              const { return SrcKey { m_func, m_last,
                                                          m_resumed }; }
  int         length()            const { return m_length; }
  bool        empty()             const { return length() == 0; }
  bool        contains(SrcKey sk) const;
  FPInvOffset initialSpOffset()   const { return m_initialSpOffset; }
  uint16_t    inlineLevel()       const { return m_inlineLevel; }
  TransID     profTransID()       const { return m_profTransID; }

  void setID(BlockId id)                  { m_id = id; }
  void setProfTransID(TransID ptid)       { m_profTransID = ptid; }
  void setInitialSpOffset(FPInvOffset sp) { m_initialSpOffset = sp; }

  /*
   * Set and get whether or not this block ends with an inlined FCall. Inlined
   * FCalls should not emit any code themselves, and they will be followed by
   * one or more blocks from the callee.
   */
  void setInlinedCallee(const Func* callee) {
    assertx(callee);
    m_inlinedCallee = callee;
  }
  const Func* inlinedCallee() const {
    return m_inlinedCallee;
  }

  /*
   * Increase the length of the Block by 1.
   */
  void addInstruction();

  /*
   * Remove all instructions after sk from the block.
   */
  void truncateAfter(SrcKey sk);

  /*
   * Add a predicted type to this block.
   *
   * Pre: sk is in the region delimited by this block.
   */
  void addPredicted(SrcKey sk, TypedLocation);

  /*
   * Add a precondition type to this block. Preconditions have no effects on
   * correctness, but entering a block with a known type that violates a
   * precondition is likely to result in a side exit after little to no
   * forward progress.
   *
   * Pre: sk is in the region delimited by this block.
   */
  void addPreCondition(SrcKey sk, TypedLocation);

  /*
   * Add information about parameter reffiness to this block.
   */
  void setParamByRef(SrcKey sk, bool);

  /*
   * Add a reffiness prediction about a pre-live ActRec.
   */
  void addReffinessPred(SrcKey, const ReffinessPred&);

  /*
   * Update the statically known Func*. It remains active until another is
   * specified, so pass nullptr to indicate that there is no longer a known
   * Func*.
   */
  void setKnownFunc(SrcKey, const Func*);

  /*
   * Set the post-conditions for this Block.
   */
  void setPostConds(const PostConditions&);

  /*
   * The following getters return references to the metadata maps holding the
   * information added using the add* and set* methods above. The best way to
   * iterate over the information is using a MapWalker, since they're all
   * backed by a sorted map.
   */
  const TypedLocMap&    typePredictions()   const { return m_typePredictions;  }
  const TypedLocMap&    typePreConditions() const { return m_typePreConditions;}
  const ParamByRefMap&  paramByRefs()       const { return m_byRefs;           }
  const RefPredMap&     reffinessPreds()    const { return m_refPreds;         }
  const KnownFuncMap&   knownFuncs()        const { return m_knownFuncs;       }
  const PostConditions& postConds()         const { return m_postConds;        }

private:
  void checkInstructions() const;
  void checkInstruction(Op op) const;
  void checkMetadata() const;

private:
  static BlockId s_nextId;

  BlockId         m_id;
  const Func*     m_func;
  const bool      m_resumed;
  const Offset    m_start;
  Offset          m_last;
  int             m_length;
  FPInvOffset     m_initialSpOffset;
  const Func*     m_inlinedCallee;
  uint16_t        m_inlineLevel; // 0 means the outer-most function
  TransID         m_profTransID;
  TypedLocMap     m_typePredictions;
  TypedLocMap     m_typePreConditions;
  ParamByRefMap   m_byRefs;
  RefPredMap      m_refPreds;
  KnownFuncMap    m_knownFuncs;
  PostConditions  m_postConds;
};

//////////////////////////////////////////////////////////////////////

/*
 * Information about the context in which we are selecting a region.
 *
 * Right now this is a source location, plus information about the
 * live types that we need to be compiling for.  There is no
 * implication that the region selected will necessarily specialize
 * for those types.
 */
struct RegionContext {
  struct LiveType;
  struct PreLiveAR;

  const Func* func;
  Offset bcOffset;
  FPInvOffset spOffset;
  bool resumed;
  jit::vector<LiveType> liveTypes;
  jit::vector<PreLiveAR> preLiveARs;
};

/*
 * Live information about the type of a local or stack slot.
 */
struct RegionContext::LiveType {
  RegionDesc::Location location;
  Type type;
};

/*
 * Pre-live ActRec information for a RegionContext.  The ActRec is
 * located stackOff slots above the stack at the start of the context,
 * contains the supplied func, and the m_this/m_class field is of type
 * objOrClass.
 */
struct RegionContext::PreLiveAR {
  int32_t stackOff;
  const Func* func;
  Type        objOrCls;
};

//////////////////////////////////////////////////////////////////////

template<class Work> inline
void RegionDesc::forEachArc(Work w) const {
  for (auto& src : m_blocks) {
    auto srcId = src->id();
    for (auto dstId : succs(srcId)) {
      w(srcId, dstId);
    }
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * Select a compilation region corresponding to the given context.
 * The shape of the region selected is controlled by
 * RuntimeOption::EvalJitRegionSelector.
 *
 * This function may return nullptr.
 *
 * For now this is hooked up in MCGenerator::translateWork, and
 * returning nullptr causes it to use the current level 0 tracelet
 * analyzer.  Eventually we'd like this to completely replace analyze.
 */
RegionDescPtr selectRegion(const RegionContext& context, TransKind kind);

/*
 * Select a compilation region based on profiling information.  This
 * is used in JitPGO mode.  Argument transId specifies the profiling
 * translation that triggered the profiling-based region selection.
 */
RegionDescPtr selectHotRegion(TransID transId,
                              MCGenerator* mcg);

/*
 * Select a compilation region as long as possible using the given context.
 * The region will be broken before the first instruction that attempts to
 * consume an input with an insufficiently precise type, or after most control
 * flow instructions.  This uses roughly the same heuristics as the old
 * analyze() framework.
 *
 * May return a null region if the given RegionContext doesn't have enough
 * information to translate at least one instruction.
 *
 * The `allowInlining' flag should be disabled when we are selecting a tracelet
 * whose shape will be analyzed by the InliningDecider.
 */
RegionDescPtr selectTracelet(const RegionContext& ctx, bool profiling,
                             bool allowInlining = true);

/*
 * Select the hottest trace beginning with triggerId.
 */
RegionDescPtr selectHotTrace(TransID triggerId,
                             const ProfData* profData,
                             TransCFG& cfg,
                             TransIDSet& selectedSet,
                             TransIDVec* selectedVec = nullptr);

/*
 * Create a region, beginning with headId, that includes as much of
 * the TransCFG as possible (in "wholecfg" mode), but that can be
 * pruned to eliminate cold/unlikely code as well (in "hotcfg" mode).
 */
RegionDescPtr selectHotCFG(TransID headId,
                           const ProfData* profData,
                           const TransCFG& cfg,
                           TransIDSet& selectedSet,
                           TransIDVec* selectedVec = nullptr);

/*
 * Checks whether the type predictions at the beginning of block
 * satisfy the post-conditions in prevPostConds.
 */
bool preCondsAreSatisfied(const RegionDesc::BlockPtr& block,
                          const TypedLocations& prevPostConds);

/*
 * This function returns true for control-flow bytecode instructions that
 * are not supported in the middle of a region yet.
 */
bool breaksRegion(SrcKey sk);

/*
 * Creates regions covering all existing profile translations for
 * func, and returns them in the regions vector.
 */
void regionizeFunc(const Func*  func,
                   MCGenerator* mcg,
                   RegionVec&   regions);

/*
 * Returns the PGO region selector to be used for the given `func'.
 * This depends on the values of RuntimeOption::EvalJitPGORegionSelector
 * and RuntimeOption::EvalJitPGOCFGHotFuncOnly and the given `func'.
 */
PGORegionMode pgoRegionMode(const Func& func);

/*
 * Functions to map BlockIds to the TransIDs used when the block was
 * profiled.
 */
bool    hasTransID(RegionDesc::BlockId blockId);
TransID getTransID(RegionDesc::BlockId blockId);

/*
 * Checks if the given region is well-formed.
 */
bool check(const RegionDesc& region, std::string& error);

/*
 * Debug stringification for various things.
 */
std::string show(RegionDesc::Location);
std::string show(RegionDesc::TypedLocation);
std::string show(const PostConditions&);
std::string show(const RegionDesc::ReffinessPred&);
std::string show(RegionContext::LiveType);
std::string show(RegionContext::PreLiveAR);
std::string show(const RegionContext&);
std::string show(const RegionDesc::Block&);
std::string show(const RegionDesc&);

//////////////////////////////////////////////////////////////////////

}}

#endif
