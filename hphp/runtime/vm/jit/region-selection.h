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
#include <boost/container/flat_map.hpp>
#include <boost/range/iterator_range.hpp>
#include <vector>

#include "folly/Format.h"

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

namespace JIT {
struct Tracelet;
struct MCGenerator;
}

namespace JIT {

using boost::container::flat_map;
using boost::container::flat_multimap;

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
  struct TypePred;
  struct ReffinessPred;
  typedef std::shared_ptr<Block> BlockPtr;
  typedef uint32_t BlockId;

  template<typename... Args>
  Block* addBlock(Args&&... args) {
    blocks.push_back(
      std::make_shared<Block>(std::forward<Args>(args)...));
    return blocks.back().get();
  }
  void addArc(BlockId src, BlockId dst);
  std::vector<BlockPtr> blocks;
  std::vector<Arc>      arcs;
};

typedef std::shared_ptr<RegionDesc>                      RegionDescPtr;
typedef std::vector<RegionDescPtr>                       RegionVec;
typedef hphp_hash_set<RegionDescPtr,
                      smart_pointer_hash<RegionDescPtr>> RegionSet;

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
    uint32_t offset;   // offset from SP
    uint32_t fpOffset; // offset from FP
  };

  /* implicit */ Location(Local l) : m_tag{Tag::Local}, m_local(l) {}
  /* implicit */ Location(Stack s) : m_tag{Tag::Stack}, m_stack(s) {}

  Tag tag() const { return m_tag; };

  uint32_t localId() const {
    assert(m_tag == Tag::Local);
    return m_local.locId;
  }

  uint32_t stackOffset() const {
    assert(m_tag == Tag::Stack);
    return m_stack.offset;
  }

  uint32_t stackOffsetFromFp() const {
    assert(m_tag == Tag::Stack);
    return m_stack.fpOffset;
  }

  bool operator==(const Location& other) const {
    return (m_tag == other.m_tag) &&
      ((m_tag == Tag::Local && localId() == other.localId()) ||
       (m_tag == Tag::Stack &&
        stackOffsetFromFp() == other.stackOffsetFromFp()));
  }

  bool operator!=(const Location& other) const {
    return !(*this == other);
  }

  bool operator<(const Location& other) const {
    return m_tag < other.m_tag ||
      (m_tag == other.m_tag && m_local.locId < other.m_local.locId);
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
 * A type prediction for somewhere in the middle of or start of a
 * region.
 *
 * All types annotated in the RegionDesc are expected to be
 * predictions, i.e. things that need to be guarded on or checked and
 * then side-exited on.  Getting the ahead-of-time static types
 * attached is handled by a different module.
 */
struct RegionDesc::TypePred {
  Location location;
  Type type;
};

inline bool operator==(const RegionDesc::TypePred& a,
                       const RegionDesc::TypePred& b) {
  return a.location == b.location && a.type == b.type;
}

typedef std::vector<RegionDesc::TypePred> PostConditions;

/*
 * A prediction for the argument reffiness of the Func for a pre-live ActRec.
 *
 * mask is a bitmask of all 1's, with one bit for each parameter being passed.
 *
 * vals is a bitmask of the same length as mask, with a 1 representing a
 * parameter that will be passed by reference and a 0 for for value.
 *
 * arSpOffset is the offset from rVmSp to the ActRec.
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
class RegionDesc::Block {
  typedef flat_multimap<SrcKey, TypePred> TypePredMap;
  typedef flat_map<SrcKey, bool> ParamByRefMap;
  typedef flat_multimap<SrcKey, ReffinessPred> RefPredMap;
  typedef flat_map<SrcKey, const Func*> KnownFuncMap;

public:
  explicit Block(const Func* func, Offset start, int length, Offset initSpOff);

  Block& operator=(const Block&) = delete;

  /*
   * Accessors for the func, unit, length (in HHBC instructions), and
   * starting SrcKey of this Block.
   */
  BlockId     id()                const { return m_id; }
  const Unit* unit()              const { return m_func->unit(); }
  const Func* func()              const { return m_func; }
  SrcKey      start()             const { return SrcKey { m_func, m_start }; }
  SrcKey      last()              const { return SrcKey { m_func, m_last }; }
  int         length()            const { return m_length; }
  bool        empty()             const { return length() == 0; }
  bool        contains(SrcKey sk) const;
  Offset      initialSpOffset()   const { return m_initialSpOffset; }

  /*
   * Set and get whether or not this block ends with an inlined FCall. Inlined
   * FCalls should not emit any code themselves, and they will be followed by
   * one or more blocks from the callee.
   */
  void setInlinedCallee(const Func* callee) {
    assert(callee);
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
  void addPredicted(SrcKey sk, TypePred);

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
   * Set the postconditions for this Block.
   */
  void setPostConditions(const PostConditions&);

  /*
   * The following getters return references to the metadata maps holding the
   * information added using the add* and set* methods above. The best way to
   * iterate over the information is using a MapWalker, since they're all
   * backed by a sorted map.
   */
  const TypePredMap& typePreds()     const { return m_typePreds; }
  const ParamByRefMap& paramByRefs() const { return m_byRefs; }
  const RefPredMap& reffinessPreds() const { return m_refPreds; }
  const KnownFuncMap& knownFuncs()   const { return m_knownFuncs; }
  const PostConditions& postConds()  const { return m_postConds; }

private:
  void checkInstructions() const;
  void checkInstruction(Op op) const;
  void checkMetadata() const;

private:
  static BlockId s_nextId;

  BlockId        m_id;
  const Func*    m_func;
  const Offset   m_start;
  Offset         m_last;
  int            m_length;
  Offset         m_initialSpOffset;
  const Func*    m_inlinedCallee;

  TypePredMap    m_typePreds;
  ParamByRefMap  m_byRefs;
  RefPredMap     m_refPreds;
  KnownFuncMap   m_knownFuncs;
  PostConditions m_postConds;
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
  Offset spOffset;
  bool inGenerator;
  smart::vector<LiveType> liveTypes;
  smart::vector<PreLiveAR> preLiveARs;
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
  uint32_t    stackOff;
  const Func* func;
  Type        objOrCls;
};

//////////////////////////////////////////////////////////////////////

/*
 * Select a compilation region corresponding to the given context.
 * The shape of the region selected is controlled by
 * RuntimeOption::EvalJitRegionSelector.  If the specified shape is
 * 'legacy', then the input argument t is used to build the region.
 *
 * This function may return nullptr.
 *
 * For now this is hooked up in MCGenerator::translateWork, and
 * returning nullptr causes it to use the current level 0 tracelet
 * analyzer.  Eventually we'd like this to completely replace analyze.
 */
RegionDescPtr selectRegion(const RegionContext& context,
                           const Tracelet* t,
                           TransKind kind);

/*
 * Select a compilation region based on profiling information.  This
 * is used in JitPGO mode.  Argument transId specifies the profiling
 * translation that triggered the profiling-based region selection.
 */
RegionDescPtr selectHotRegion(TransID transId,
                              MCGenerator* mcg);

/*
 * Select a compilation region using roughly the same heuristics as the old
 * analyze() framework.
 */
RegionDescPtr selectTracelet(const RegionContext& ctx, int inlineDepth,
                             bool profiling);

/*
 * Create a compilation region corresponding to a tracelet created by
 * the old analyze() framework.
 */
RegionDescPtr selectTraceletLegacy(Offset initSpOffset,
                                   const Tracelet& tlet);

/*
 * Checks whether the type predictions at the beginning of block
 * satisfy the post-conditions in prevPostConds.
 */
bool preCondsAreSatisfied(const RegionDesc::BlockPtr& block,
                          const PostConditions& prevPostConds);

/*
 * Creates regions covering all existing profile translations for
 * func, and returns them in the regions vector.
 */
void regionizeFunc(const Func*  func,
                   MCGenerator* mcg,
                   RegionVec&   regions);

/*
 * Compare the two regions. If they differ in any way other than a being longer
 * than b, trace both regions.
 */
void diffRegions(const RegionDesc& a, const RegionDesc& b);

/*
 * Debug stringification for various things.
 */
std::string show(RegionDesc::Location);
std::string show(RegionDesc::TypePred);
std::string show(const RegionDesc::ReffinessPred&);
std::string show(RegionContext::LiveType);
std::string show(RegionContext::PreLiveAR);
std::string show(const RegionContext&);
std::string show(const RegionDesc::Block&);
std::string show(const RegionDesc::Arc&);
std::string show(const RegionDesc&);

//////////////////////////////////////////////////////////////////////

}}

#endif
