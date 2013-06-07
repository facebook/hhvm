/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include <boost/range/iterator_range.hpp>

#include "folly/Format.h"

#include "hphp/runtime/base/memory/smart_containers.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP { namespace JIT {

//////////////////////////////////////////////////////////////////////

/*
 * RegionDesc is a description of a code region.
 *
 * It consists of a list of unique SrcKey ranges, with type
 * annotations that may come from profiling or other sources.
 *
 * The first block is the entry point, and the remaining blocks must
 * be sorted in a reverse post order.
 */
struct RegionDesc {
  struct Block;
  struct Location;
  struct TypePred;
  typedef smart::unique_ptr<Block>::type BlockPtr;

  smart::vector<BlockPtr> blocks;
};
typedef smart::unique_ptr<RegionDesc>::type RegionDescPtr;

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
  struct Stack { uint32_t offset; };

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

private:
  Tag m_tag;
  union {
    Local m_local;
    Stack m_stack;
  };
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

/*
 * A basic block in the region, with type predictions for conditions
 * at various execution points, including at entry to the block.
 */
class RegionDesc::Block {
  typedef smart::vector<std::pair<SrcKey,TypePred>> TypePredList;

public:
  explicit Block(const Func* func, Offset start, int length)
    : m_func(func)
    , m_start(start)
    , m_length(length)
  {
    if (debug) checkInvariants();
  }

  Block(const Block&) = delete;
  Block& operator=(const Block&) = delete;

  /*
   * Accessors for the func, unit, length (in HHBC instructions), and
   * starting SrcKey of this Block.
   */
  const Unit* unit() const { return m_func->unit(); }
  const Func* func() const { return m_func; }
  SrcKey start() const { return SrcKey { m_func, m_start }; }
  int length() const { return m_length; }

  /*
   * Add a predicted type to this block.
   *
   * Pre: sk is in the region delimited by this block.
   */
  void addPredicted(SrcKey sk, TypePred);

  /*
   * Obtain a range for the type predictions on this block.  The
   * elements in the range are listed in ascending SrcKey order.
   *
   * The caller should assume it is a SinglePassReadableRange.
   */
  boost::iterator_range<TypePredList::const_iterator> typePreds() const {
    return boost::make_iterator_range(m_predTypes.begin(), m_predTypes.end());
  }

private:
  void checkInvariants() const;
  static bool typePredListCmp(TypePredList::const_reference,
                              TypePredList::const_reference);

private:
  const Func*  m_func;
  const Offset m_start;
  const int    m_length;
  TypePredList m_predTypes;
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
  Offset offset;
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
 * Define a compilation region that starts with sk.
 *
 * May return nullptr.
 *
 * For now this is hooked up in TranslatorX64::createTranslation, and
 * returning nullptr causes it to use the current level 0 tracelet
 * analyzer.  Eventually we'd like analyze to occur underneath this as
 * well.
 */
RegionDescPtr selectRegion(const RegionContext&);

/*
 * Debug stringification for various things.
 */
std::string show(RegionDesc::Location);
std::string show(RegionDesc::TypePred);
std::string show(RegionContext::LiveType);
std::string show(RegionContext::PreLiveAR);
std::string show(const RegionDesc::Block&);
std::string show(const RegionDesc&);

//////////////////////////////////////////////////////////////////////

}}

#endif
