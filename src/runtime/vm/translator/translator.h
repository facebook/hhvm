/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef _TRANSLATOR_H_
#define _TRANSLATOR_H_

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <map>
#include <vector>
#include <set>

#include <util/hash.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/translator/immstack.h>
#include <runtime/vm/translator/runtime-type.h>
#include <runtime/base/md5.h>

/* Translator front-end. */
namespace HPHP {
namespace VM {
namespace Transl {

typedef unsigned char* TCA; // "Translation cache adddress."

using std::vector;

/*
 * A SrcKey is a logical source instruction, currently a unit/instruction pair.
 * The units are identified by contents rather than Unit; Unit's are
 * ephemeral, and we want to reuse SrcKey's when we encounter Unit's with
 * the same contents.
 */
struct SrcKey {
  MD5 m_md5;
  Offset m_offset;

  SrcKey() :
    m_md5("00000000000000000000000000000000"),
    m_offset(0) { }

  SrcKey(const Unit *u, Offset off) :
    m_md5(u->md5()), m_offset(off) { }

  SrcKey(const Unit *u, const Opcode *i) :
    m_md5(u->md5()), m_offset(u->offsetOf(i)) { }

  int cmp(const SrcKey &r) const {
    // Can't use memcmp because of pad bytes. Frowny.
#define CMP(field) \
    if (field < r.field) return -1; \
    if (field > r.field) return 1
    CMP(m_md5);
    CMP(m_offset);
#undef CMP
    return 0;
  }
  bool operator==(const SrcKey& r) const {
    return cmp(r) == 0;
  }
  bool operator!=(const SrcKey& r) const {
    return cmp(r) != 0;
  }
  bool operator<(const SrcKey& r) const {
    return cmp(r) < 0;
  }
  bool operator>(const SrcKey& r) const {
    return cmp(r) < 0;
  }
  // Hash function
  size_t operator()(const SrcKey &sk) const {
    return HPHP::hash_int64_pair(sk.m_md5.hash(), uint64(sk.m_offset));
  }
  void trace(const char *fmt, ...) const;
  int offset() const {
    return m_offset;
  }
  void advance(const Unit* u) {
    m_offset += instrLen(u->at(offset()));
  }
};

#define SKTRACE(level, sk, ...) \
  ONTRACE(level, (sk).trace(__VA_ARGS__))

class NormalizedInstruction;

// A DynLocation is a Location-in-execution: a location, along with
// whatever is known about its runtime type.
struct DynLocation {
  Location    location;
  RuntimeType rtt;
  NormalizedInstruction* source;

  DynLocation(Location l, DataType t) : location(l), rtt(t), source(NULL) {}

  DynLocation(Location l, RuntimeType t) : location(l), rtt(t), source(NULL) {}

  DynLocation() : location(), rtt(KindOfInvalid), source(NULL) {}

  bool operator==(const DynLocation& r) const {
    return rtt == r.rtt && location == r.location;
  }

  // Hash function
  size_t operator()(const DynLocation &dl) const {
    uint64 rtthash = rtt(rtt);
    uint64 locHash = location(location);
    return rtthash ^ locHash;
  }

  std::string pretty() const {
    return Trace::prettyNode("DynLocation", location, rtt);
  }

  // Punch through a bunch of frequently called rtt and location methods.
  // While this is unlovely here, we use DynLocation in bazillions of
  // places in the translator, and constantly saying ".rtt" is worse.
  bool isString() const {
    return rtt.isString();
  }
  bool isInt() const {
    return rtt.isInt();
  }
  bool isVariant() const {
    return rtt.isVariant();
  }
  bool isValue() const {
    return rtt.isValue();
  }
  DataType valueType() const {
    return rtt.valueType();
  }
  DataType outerType() const {
    return rtt.outerType();
  }

  bool isStack() const {
    return location.space == Location::Stack;
  }
  bool isLocal() const {
    return location.space == Location::Local;
  }

  // Uses the runtime state. True if this dynLocation can be overwritten by SetG's
  // and SetM's.
  bool canBeAliased() const;
};

// A NormalizedInstruction has been decorated with its typed inputs and
// outputs.
class NormalizedInstruction {
 public:
  static const int kNoImm = -1;

  NormalizedInstruction* next;
  NormalizedInstruction* prev;

  SrcKey source;
  const Func* funcd; // The Func in the topmost AR on the stack. Guaranteed to
                     // be accurate. Don't guess about this. Note that this is
                     // *not* the function whose body the NI belongs to.
  const Unit* m_unit;
  vector<DynLocation*> inputs;
  DynLocation* outStack;
  DynLocation* outLocal;
  vector<DynLocation*> inputHomes;
  vector<Location> deadLocs; // locations that die at the end of this
                             // instruction
  ArgUnion imm[2];
  ImmVector* immVecPtr; // pointer to the vector immediate, or NULL if the
                        // instruction has no vector immediate
  // StackOff: logical delta at *start* of this instruction to
  // stack at tracelet entry.
  int stackOff;
  bool breaksBB;
  bool preppedByRef;    // For Prep*
  int constImmPos;
  ArgUnion constImm;
  bool txSupported;

  Opcode op() const;
  PC pc() const;
  const Unit* unit() const;
  Offset offset() const;

  NormalizedInstruction() :
    next(NULL),
    prev(NULL),
    source(),
    inputs(),
    outStack(NULL),
    outLocal(NULL),
    inputHomes(),
    deadLocs(),
    immVecPtr(NULL),
    constImmPos(kNoImm),
    txSupported(false)
  { }
};

class TranslationFailedExc : public std::exception {
 public:
  const char* m_file; // must be static
  const int m_line;
  TranslationFailedExc(const char* file, int line) :
    m_file(file), m_line(line) { }
};

class UnknownInputExc : public std::exception {
 public:
  const char* m_file; // must be static
  const int m_line;
  UnknownInputExc(const char* file, int line)
    : m_file(file), m_line(line) { }
};

#define punt() do { \
  throw TranslationFailedExc(__FILE__, __LINE__); \
} while(0)

#define puntUnless(predicate) do { \
  if (!(predicate)) { \
    punt(); \
  } \
} while(0)

#define throwUnknownInput() do { \
  throw UnknownInputExc(__FILE__, __LINE__); \
} while(0);

/*
 * A tracelet is a unit of input to the back-end. It is a partially typed,
 * non-maximal basic block, representing the next slice of the program to
 * be executed.
 * It is a consecutive set of instructions, only the last of which may be a
 * transfer of control, annotated types and locations for each opcode's input
 * and output.
 */
typedef hphp_hash_map<Location, DynLocation*, Location> ChangeMap;
typedef ChangeMap DepMap;
typedef hphp_hash_set<Location, Location> LocationSet;

struct ClassDep {
  Class*   m_class; // Requires: m_loc is sub-class of m_class ...
  int      m_depth; // at depth d in the inheritance hierarchy.
};
typedef hphp_hash_map<Location, ClassDep, Location> ClassDeps;

struct InstrStream {
  InstrStream() : first(NULL), last(NULL) {}
  void append(NormalizedInstruction* ni);
  void remove(NormalizedInstruction* ni);
  NormalizedInstruction* first;
  NormalizedInstruction* last;
};

struct RefDeps {
  vector<bool> m_mask;
  vector<bool> m_vals;

  RefDeps() {
    ASSERT(m_mask.size() == 0);
    ASSERT(m_vals.size() == 0);
  }

  void addDep(unsigned argNum, bool isRef) {
    if (argNum >= m_mask.size()) {
      ASSERT(argNum >= m_vals.size());
      m_mask.resize(argNum + 1);
      m_vals.resize(argNum + 1);
    }
    m_mask[argNum] = true;
    m_vals[argNum] = isRef;
  }
};

struct ActRecState {
  // State for tracking function param reffiness. m_topFunc is the function
  // for the activation record that is closest to the top of the stack, or
  // NULL if it is currently unknown. A tracelet can be in one of three
  // epistemological states: GUESSED, KNOWN, and UNKNOWABLE. We start out in
  // GUESSED, with m_topFunc == NULL (not yet guessed); when it's time to
  // guess, we will use the ActRec seen on the top of stack at compilation
  // time as a hint for refs going forward.
  //
  // The KNOWN state is a very strong guarantee. It means that no matter when
  // this tracelet is executed, no matter what else has happened, the ActRec
  // closest to the top of the stack WILL contain m_topFunc. This means: if that
  // function is defined conditionally, or defined in some other module, you
  // cannot correctly make that assertion. KNOWN indicates absolute certainty
  // about all possible futures.
  //
  // This strange "not-guessed-yet-but-could" state is required by our
  // VM design; at present, the ActRec is not easily recoverable from an
  // arbitrary instruction boundary. However, it can be recovered from the
  // instructions that need to do so.
  enum {
    GUESSABLE, KNOWN, UNKNOWABLE
  }              m_state;
  const Func*    m_topFunc;
  static const int InvalidEntryArDelta = INT_MAX;
  int            m_entryArDelta; // delta at BB entry to guessed ActRec.

  ActRecState();
  void pushFuncD(const Func* func);
  void pushDynFunc(void);
  bool getReffiness(int argNum, int stackOffset, RefDeps* outRefDeps);
};

struct Tracelet {
  ChangeMap      m_changes;
  DepMap         m_dependencies;
  InstrStream    m_instrStream;
  int            m_stackChange;
  SrcKey         m_sk;

  // numOpcodes is the number of raw opcode instructions, before optimiation.
  // The immediates optimization may both:
  //
  // 1. remove the first opcode, thus making
  //        sk.instr != instrs.first->source.instr
  // 2. remove no longer needed instructions
  int            m_numOpcodes;

  // Assumptions about entering actRec's reffiness.
  ActRecState    m_arState;
  RefDeps        m_refDeps;

  // Assumptions about object class lineage.
  ClassDeps      m_classDeps;

  /*
   * If we were unable to make sense of the instruction stream (e.g., it
   * used instructions that the translator does not understand), then this
   * tracelet is useful only for defining the boundaries of a basic block.
   * The low-level translator can handle this by backing off to the
   * bytecode interpreter.
   */
  bool           m_analysisFailed;

  // Track which NormalizedInstructions and DynLocations are owned by this
  // Tracelet; used for cleanup purposes
  std::vector<NormalizedInstruction*> m_instrs;
  std::vector<DynLocation*> m_dynlocs;

  Tracelet() :
    m_stackChange(0),
    m_arState(),
    m_analysisFailed(false) { }
  ~Tracelet();

  NormalizedInstruction* newNormalizedInstruction();
  DynLocation* newDynLocation(Location l, DataType t);
  DynLocation* newDynLocation(Location l, RuntimeType t);
  DynLocation* newDynLocation();
};

struct TraceletContext {
  Tracelet*   m_t;
  ChangeMap   m_currentMap;
  DepMap      m_dependencies;
  LocationSet m_changeSet;
  LocationSet m_deletedSet;
  bool        m_aliasTaint;

  TraceletContext() : m_t(NULL), m_aliasTaint(false)  {}
  TraceletContext(Tracelet* t) : m_t(t), m_aliasTaint(false) {}
  DynLocation* recordRead(const Location& l);
  void recordWrite(DynLocation* dl, NormalizedInstruction* source);
  void recordDelete(const Location& l);
  void aliasTaint();

 private:
  static bool canBeAliased(const DynLocation* dl);
};

/*
 * For debugging purposes, a record of the place a txn came from. Record
 * the start address of each translation here. Useful for logs,
 * post-mortems, etc.
 */
struct TransRec {
  SrcKey src;
  bool isAnchor;
  vector<DynLocation> dependencies;
  TransRec() { }
  TransRec(SrcKey s) : src(s), isAnchor(true) { }
  TransRec(SrcKey s, const Tracelet& t) : src(s), isAnchor(false) {
    for (DepMap::const_iterator dep = t.m_dependencies.begin();
         dep != t.m_dependencies.end();
         ++dep) {
      dependencies.push_back(*dep->second);
    }
  }
};

/*
 * Translator annotates a tracelet with input/output locations/types.
 */
class Translator {
public:
  // kFewLocals is a magic value used to decide whether or not to
  // generate specialized code for RetC
  static const int kFewLocals = 4;

private:
  friend struct TraceletContext;

  int stackFrameOffset; // sp at current instr; used to normalize

  void getInputs(Tracelet& t,
                 NormalizedInstruction* ni,
                 int& currentStackOffset,
                 vector<Location>& inputs);
  void getOutputs(Tracelet& t,
                  NormalizedInstruction* ni,
                  int& currentStackOffset,
                  bool& writeAlias);

  static Location tvToLocation(const TypedValue* tv);
  static RuntimeType liveType(Location l, const Unit &u);
  static RuntimeType liveType(const Cell* outer, const Location& l);

  void consumeStackEntry(Tracelet* tlet, NormalizedInstruction* ni);
  void produceStackEntry(Tracelet* tlet, NormalizedInstruction* ni);
  void produceDataRef(Tracelet* tlet, NormalizedInstruction* ni,
                                      Location loc);

  void findImmable(ImmStack &stack, NormalizedInstruction* ni);

protected:
  void requestResetHighLevelTranslator();

public:

  Translator();
  virtual ~Translator() { }
  static Translator* Get();

  /*
   * Interface between the arch-dependent translator and outside world.
   */
  virtual void processInit() = 0;
  virtual void requestInit() = 0;
  virtual void requestExit() = 0;
  virtual void analyzeInstr(Tracelet& t, NormalizedInstruction& i) = 0;
  virtual TCA funcPrologue(const Func* f, int nArgs, int flags) = 0;
  virtual TCA getCallToExit() = 0;
  virtual TCA getRetFromInterpretedFrame() = 0;
  virtual void resume(SrcKey sk) = 0;
  virtual void defineCns(StringData* name) = 0;

  enum {
    FuncPrologueNormal      = 0,
    FuncPrologueMagicCall   = 1,
    FuncPrologueIntercepted = 2,
  };

  typedef std::map<TCA, TransRec> TransDB;
  virtual TransDB& getTransDB() = 0;

  Tracelet analyze(const SrcKey* sk);
  void advance(Opcode const **instrs);
  static int locPhysicalOffset(Location l, const Func* f = NULL);
  static bool typeIsString(DataType type) {
    return type == KindOfString || type == KindOfStaticString;
  }
  static bool liveFrameIsPseudoMain();
};

/*
 * opcodeBreaksBB --
 *
 *   Instructions which always break a basic block. Mostly branches.
 */
static inline bool
opcodeBreaksBB(const Opcode instr) {
  switch (instr) {
    case OpJmp:
    case OpJmpZ:
    case OpJmpNZ:
    case OpFCall:
    case OpRetC:
    case OpRetV:
    case OpRaise:
    case OpExit:
    case OpFatal:
    case OpIterNext:
    case OpIterInit: // May branch to fail case.
    case OpIterInitM: // May branch to fail case.
    case OpThrow:
    case OpUnwind:
    case OpIncl:
    case OpInclOnce:
    case OpReq:
    case OpReqOnce:
    case OpEval:
      return true;
    default:
      return false;
  }
}

} } } // HPHP::VM::Transl

#endif
