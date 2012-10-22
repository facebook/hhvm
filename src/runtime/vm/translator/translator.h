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

#include <boost/ptr_container/ptr_vector.hpp>
#include <util/hash.h>
#include <runtime/base/execution_context.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/translator/immstack.h>
#include <runtime/vm/translator/runtime-type.h>
#include <runtime/vm/translator/fixup.h>
#include <runtime/vm/translator/writelease.h>
#include <runtime/vm/translator/trans-data.h>
#include <runtime/vm/debugger_hook.h>
#include <runtime/base/md5.h>

/* Translator front-end. */
namespace HPHP {
namespace VM {
namespace Transl {

static const bool trustSigSegv = false;

static const uint32 transCountersPerChunk = 1024 * 1024 / 8;

class TranslatorX64;
extern TranslatorX64* volatile nextTx64;
extern __thread TranslatorX64* tx64;

/*
 * REGSTATE_DIRTY when the live register state is spread across the
 * stack and m_fixup, REGSTATE_CLEAN when it has been sync'ed into
 * g_context.
 */
enum VMRegState {
  REGSTATE_CLEAN,
  REGSTATE_DIRTY
};
extern __thread VMRegState tl_regState;

/*
 * A SrcKey is a logical source instruction, currently a unit/instruction pair.
 * The units are identified by contents rather than Unit; Unit's are
 * ephemeral, and we want to reuse SrcKey's when we encounter Unit's with
 * the same contents.
 */
struct SrcKey {
  Func::FuncId m_funcId;
  Offset m_offset;

  SrcKey() : m_funcId(Func::InvalidId), m_offset(0) { }

  SrcKey(const Func* f, Offset off) :
    m_funcId(f->getFuncId()), m_offset(off) { }

  SrcKey(const Func* f, const Opcode* i) :
    m_funcId(f->getFuncId()), m_offset(f->unit()->offsetOf(i)) { }

  int cmp(const SrcKey &r) const {
    // Can't use memcmp because of pad bytes. Frowny.
#define CMP(field) \
    if (field < r.field) return -1; \
    if (field > r.field) return 1
    CMP(m_funcId);
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
    return cmp(r) > 0;
  }
  // Hash function for both hash_map and tbb conventions.
  static size_t hash(const SrcKey &sk) {
    return HPHP::hash_int64_pair(sk.m_funcId, uint64(sk.m_offset));
  }
  size_t operator()(const SrcKey& sk) const {
    return hash(sk);
  }
  static bool equal(const SrcKey& sk1, const SrcKey& sk2) {
    return sk1 == sk2;
  }

  // Packed representation of SrcKeys for use in contexts where we
  // want atomicity.  (SrcDB.)
  typedef uint64_t AtomicInt;

  AtomicInt toAtomicInt() const {
    return uint64_t(m_funcId) << 32 | uint64_t(m_offset);
  }

  static SrcKey fromAtomicInt(AtomicInt in) {
    SrcKey k;
    k.m_funcId = in >> 32;
    k.m_offset = in & 0xffffffff;
    return k;
  }

  void trace(const char *fmt, ...) const;
  int offset() const {
    return m_offset;
  }
  void advance(const Unit* u) {
    m_offset += instrLen(u->at(offset()));
  }
};

typedef hphp_hash_set<SrcKey, SrcKey> SrcKeySet;
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
  bool isDouble() const {
    return rtt.isDouble();
  }
  bool isBoolean() const {
    return rtt.isBoolean();
  }
  bool isVariant() const {
    return rtt.isVariant();
  }
  bool isValue() const {
    return rtt.isValue();
  }
  bool isNull() const {
    return rtt.isNull();
  }
  bool isObject() const {
    return rtt.isObject();
  }
  bool isArray() const {
    return rtt.isArray();
  }
  DataType valueType() const {
    return rtt.valueType();
  }
  DataType outerType() const {
    return rtt.outerType();
  }

  bool isStack() const {
    return location.isStack();
  }
  bool isLocal() const {
    return location.isLocal();
  }
  bool isLiteral() const {
    return location.isLiteral();
  }

  // Uses the runtime state. True if this dynLocation can be overwritten by
  // SetG's and SetM's.
  bool canBeAliased() const;
};

// Flags that summarize the plan for handling a given instruction.
enum TXFlags {
  Interp = 0,       // default; must be boolean false
  Supported = 1,    // Not interpreted, though possibly with C++
  NonReentrant = 2, // Supported with no possibility of reentry.
  MachineCode  = 4, // Supported without C++ at all.
  Simple = NonReentrant | Supported,
  Native = MachineCode | Simple
};

// A NormalizedInstruction has been decorated with its typed inputs and
// outputs.
class NormalizedInstruction {
 public:
  NormalizedInstruction* next;
  NormalizedInstruction* prev;

  SrcKey source;
  const Func* funcd; // The Func in the topmost AR on the stack. Guaranteed to
                     // be accurate. Don't guess about this. Note that this is
                     // *not* the function whose body the NI belongs to.
                     // Note that for an FPush* may be set to the (statically
                     // known Func* that /this/ instruction is pushing)
  const StringData* funcName;
    // For FCall's, an opaque identifier that is either null, or uniquely
    // identifies the (functionName, -arity) pair of this call site.
  const Unit* m_unit;
  vector<DynLocation*> inputs;
  DynLocation* outStack;
  DynLocation* outLocal;
  DynLocation* outStack2; // Used for CGetL2
  DynLocation* outStack3; // Used for CGetL3
  vector<Location> deadLocs; // locations that die at the end of this
                             // instruction
  ArgUnion imm[3];
  ImmVector immVec; // vector immediate; will have !isValid() if the
                    // instruction has no vector immediate

  // The member codes for the M-vector.
  std::vector<MemberCode> immVecM;

  /*
   * For property dims, if we know the Class* for the base when we'll
   * be executing a given dim, it is stored here (at the index for the
   * relevant member code minus 1, because the known class for the
   * first member code is given by the base in inputs[]).
   *
   * Other entries here store null.  See MetaInfo::MVecPropClass.
   */
  std::vector<Class*> immVecClasses;

  unsigned checkedInputs;
  // StackOff: logical delta at *start* of this instruction to
  // stack at tracelet entry.
  int stackOff;
  unsigned hasConstImm:1;
  unsigned startsBB:1;
  unsigned breaksTracelet:1;
  unsigned changesPC:1;
  unsigned fuseBranch:1;
  unsigned preppedByRef:1;    // For FPass*; indicates parameter reffiness
  unsigned manuallyAllocInputs:1;
  unsigned invertCond:1;
  unsigned outputPredicted:1;
  unsigned ignoreInnerType:1;
  /*
   * skipSync indicates that a previous instruction that should have
   * adjusted the stack (eg FCall, Req*) didnt, because it could see
   * that the next one was going to immediately adjust it again
   * (ie at this point, rVmSp holds the "correct" value, rather
   *  than the value it had at the beginning of the tracelet)
   */
  unsigned skipSync:1;
  /*
   * grouped indicates that the tracelet should not be broken
   * (eg by a side exit) between the preceding instruction and
   * this one
   */
  unsigned grouped:1;
  /*
   * guardedThis indicates that we know that ar->m_this is
   * a valid $this. eg:
   *
   *   $this->foo = 1; # needs to check that $this is non-null
   *   $this->bar = 2; # can skip the check
   *   return 5;       # can decRef ar->m_this unconditionally
   */
  unsigned guardedThis:1;
  /*
   * guardedCls indicates that we know the class exists
   */
  unsigned guardedCls:1;
  /*
   * dont check the surprise flag
   */
  unsigned noSurprise:1;
  /*
    noCtor is set on FPushCtorD to say that the ctor is
    going to be skipped (so dont setup an actrec)
  */
  unsigned noCtor:1;
  /*
   * instruction is statically known to have no effect, e.g. unboxing a Cell
   */
  unsigned noOp:1;
  /*
   * This is an FPush* that will be directly bound to a Func*
   */
  unsigned directCall:1;

  ArgUnion constImm;
  TXFlags m_txFlags;

  Op op() const;
  Op mInstrOp() const;
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
    outStack2(NULL),
    outStack3(NULL),
    deadLocs(),
    checkedInputs(0),
    hasConstImm(false),
    invertCond(false),
    ignoreInnerType(false),
    skipSync(false),
    grouped(false),
    guardedThis(false),
    guardedCls(false),
    noSurprise(false),
    noCtor(false),
    noOp(false),
    directCall(false),
    m_txFlags(Interp) {
    memset(imm, 0, sizeof(imm));
  }

  bool isJmpNZ() const {
    ASSERT(op() == OpJmpNZ || op() == OpJmpZ);
    return (op() == OpJmpNZ) != invertCond;
  }

  bool isSupported() const {
    return (m_txFlags & Supported) == Supported;
  }

  bool isSimple() const {
    return (m_txFlags & Simple) == Simple;
  }

  bool isNative() const {
    return (m_txFlags & Native) == Native;
  }

  void markInputInferred(int i) {
    if (i < 32) checkedInputs |= 1u << i;
  }

  bool inputWasInferred(int i) const {
    return i < 32 && ((checkedInputs >> i) & 1);
  }

  enum OutputUse {
    OutputUsed,
    OutputUnused,
    OutputInferred,
    OutputDoesntCare
  };
  OutputUse outputIsUsed(DynLocation* output) const;

  std::string toString() const;
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

struct InstrStream {
  InstrStream() : first(NULL), last(NULL) {}
  void append(NormalizedInstruction* ni);
  void remove(NormalizedInstruction* ni);
  NormalizedInstruction* first;
  NormalizedInstruction* last;
};

struct RefDeps {
  struct Record {
    vector<bool> m_mask;
    vector<bool> m_vals;

    std::string pretty() const {
      std::ostringstream out;
      out << "mask=";
      for (size_t i = 0; i < m_mask.size(); ++i) {
        out << (m_mask[i] ? "1" : "0");
      }
      out << " vals=";
      for (size_t i = 0; i < m_vals.size(); ++i) {
        out << (m_vals[i] ? "1" : "0");
      }
      return out.str();
    }
  };
  typedef hphp_hash_map<int64, Record, int64_hash> ArMap;
  ArMap m_arMap;

  RefDeps() {}

  void addDep(int entryArDelta, unsigned argNum, bool isRef) {
    if (m_arMap.find(entryArDelta) == m_arMap.end()) {
      m_arMap[entryArDelta] = Record();
    }
    Record& r = m_arMap[entryArDelta];
    if (argNum >= r.m_mask.size()) {
      ASSERT(argNum >= r.m_vals.size());
      r.m_mask.resize(argNum + 1);
      r.m_vals.resize(argNum + 1);
    }
    r.m_mask[argNum] = true;
    r.m_vals[argNum] = isRef;
  }

  size_t size() const {
    return m_arMap.size();
  }
};

struct ActRecState {
  // State for tracking function param reffiness. m_topFunc is the function
  // for the activation record that is closest to the top of the stack, or
  // NULL if it is currently unknown. A tracelet can be in one of three
  // epistemological states: GUESSABLE, KNOWN, and UNKNOWABLE. We start out in
  // GUESSABLE, with m_topFunc == NULL (not yet guessed); when it's time to
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
  static const int InvalidEntryArDelta = INT_MAX;

  enum State {
    GUESSABLE, KNOWN, UNKNOWABLE
  };

  struct Record {
    State          m_state;
    const Func*    m_topFunc;
    int            m_entryArDelta; // delta at BB entry to guessed ActRec.
  };

  std::vector<Record> m_arStack;

  ActRecState() {}
  void pushFuncD(const Func* func);
  void pushDynFunc();
  void pop();
  bool getReffiness(int argNum, int stackOffset, RefDeps* outRefDeps);
  const Func* getCurrentFunc();
  State getCurrentState();
};

struct Tracelet : private boost::noncopyable {
  ChangeMap      m_changes;
  DepMap         m_dependencies;
  DepMap         m_resolvedDeps; // dependencies resolved by static analysis
  InstrStream    m_instrStream;
  int            m_stackChange;

  // SrcKey for the start of the Tracelet.  This might be different
  // from m_instrStream.first->source.
  SrcKey         m_sk;

  // After this Tracelet runs, this is the SrcKey for the next
  // instruction that we should go to.  Note that this can be
  // different from m_instrStream.last->source.advance() if we removed
  // the last instruction from the stream.
  SrcKey         m_nextSk;

  // numOpcodes is the number of raw opcode instructions, before optimization.
  // The immediates optimization may both:
  //
  // 1. remove the first opcode, thus making
  //        sk.instr != instrs.first->source.instr
  // 2. remove no longer needed instructions
  int            m_numOpcodes;

  // Assumptions about entering actRec's reffiness.
  ActRecState    m_arState;
  RefDeps        m_refDeps;

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
  boost::ptr_vector<NormalizedInstruction> m_instrs;
  boost::ptr_vector<DynLocation> m_dynlocs;

  Tracelet() :
    m_stackChange(0),
    m_arState(),
    m_analysisFailed(false) { }

  NormalizedInstruction* newNormalizedInstruction();
  DynLocation* newDynLocation(Location l, DataType t);
  DynLocation* newDynLocation(Location l, RuntimeType t);
  DynLocation* newDynLocation();

  void print();
};

struct TraceletContext {
  Tracelet*   m_t;
  ChangeMap   m_currentMap;
  DepMap      m_dependencies;
  DepMap      m_resolvedDeps; // dependencies resolved by static analysis
  LocationSet m_changeSet;
  LocationSet m_deletedSet;
  bool        m_aliasTaint;
  bool        m_varEnvTaint;

  TraceletContext()
    : m_t(NULL), m_aliasTaint(false), m_varEnvTaint(false) {}
  TraceletContext(Tracelet* t)
    : m_t(t), m_aliasTaint(false), m_varEnvTaint(false) {}
  DynLocation* recordRead(const InputInfo& l, bool useHHIR,
                          DataType staticType = KindOfInvalid);
  void recordWrite(DynLocation* dl, NormalizedInstruction* source);
  void recordDelete(const Location& l);
  void aliasTaint();
  void varEnvTaint();

 private:
  static bool canBeAliased(const DynLocation* dl);
};

enum TransKind {
  TransNormal = 0,
  TransAnchor = 1,
  TransProlog = 2,
};

const char* getTransKindName(TransKind kind);

/*
 * Used to maintain a mapping from the bytecode to its corresponding x86.
 */
struct TransBCMapping {
  Offset bcStart;
  TCA    aStart;
  TCA    astubsStart;
};

/*
 * A record with various information about a translation.
 */
struct TransRec {
  TransID                 id;
  TransKind               kind;
  SrcKey                  src;
  MD5                     md5;
  uint32                  bcStopOffset;
  vector<DynLocation>     dependencies;
  TCA                     aStart;
  uint32                  aLen;
  TCA                     astubsStart;
  uint32                  astubsLen;
  TCA                     counterStart;
  uint8                   counterLen;
  vector<TransBCMapping>  bcMapping;

  static const TransID InvalidID = -1LL;

  TransRec() {}

  TransRec(SrcKey    s,
           MD5       _md5,
           TransKind _kind,
           TCA       _aStart = 0,
           uint32    _aLen = 0,
           TCA       _astubsStart = 0,
           uint32    _astubsLen = 0) :
      id(0), kind(_kind), src(s), md5(_md5), bcStopOffset(0),
      aStart(_aStart), aLen(_aLen),
      astubsStart(_astubsStart), astubsLen(_astubsLen),
      counterStart(0), counterLen(0) { }

  TransRec(SrcKey                   s,
           MD5                      _md5,
           const Tracelet&          t,
           TCA                      _aStart = 0,
           uint32                   _aLen = 0,
           TCA                      _astubsStart = 0,
           uint32                   _astubsLen = 0,
           TCA                      _counterStart = 0,
           uint8                    _counterLen = 0,
           vector<TransBCMapping>   _bcMapping = vector<TransBCMapping>()) :
      id(0), kind(TransNormal), src(s), md5(_md5),
      bcStopOffset(t.m_nextSk.offset()), aStart(_aStart), aLen(_aLen),
      astubsStart(_astubsStart), astubsLen(_astubsLen),
      counterStart(_counterStart), counterLen(_counterLen),
      bcMapping(_bcMapping) {
    for (DepMap::const_iterator dep = t.m_dependencies.begin();
         dep != t.m_dependencies.end();
         ++dep) {
      dependencies.push_back(*dep->second);
    }
  }

  void setID(TransID newID) { id = newID; }
  string print(uint64 profCount) const;
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

  void analyzeSecondPass(Tracelet& t);
  bool applyInputMetaData(Unit::MetaHandle&,
                          NormalizedInstruction* ni,
                          TraceletContext& tas,
                          InputInfos& ii);
  void getInputs(Tracelet& t,
                 NormalizedInstruction* ni,
                 int& currentStackOffset,
                 InputInfos& inputs);
  void getOutputs(Tracelet& t,
                  NormalizedInstruction* ni,
                  int& currentStackOffset,
                  bool& varEnvTaint);

  static RuntimeType liveType(Location l, const Unit &u);
  static RuntimeType liveType(const Cell* outer, const Location& l);

  void consumeStackEntry(Tracelet* tlet, NormalizedInstruction* ni);
  void produceStackEntry(Tracelet* tlet, NormalizedInstruction* ni);
  void produceDataRef(Tracelet* tlet, NormalizedInstruction* ni,
                                      Location loc);

  void findImmable(ImmStack &stack, NormalizedInstruction* ni);

  virtual void syncWork() = 0;
  virtual void invalidateSrcKey(const SrcKey& sk) = 0;

protected:
  void requestResetHighLevelTranslator();

  TCA m_resumeHelper;

  typedef std::map<TCA, TransID> TransDB;
  TransDB            m_transDB;
  vector<TransRec>   m_translations;
  vector<uint64*>    m_transCounters;

  // For HHIR-based translation
  bool               m_useHHIR;

  static Lease s_writeLease;
  static volatile bool s_replaceInFlight;

public:

  Translator();
  virtual ~Translator();
  static Translator* Get();
  static Lease& WriteLease() {
    return s_writeLease;
  }
  static bool ReplaceInFlight() {
    return s_replaceInFlight;
  }
  static RuntimeType outThisObjectType();

  /*
   * Interface between the arch-dependent translator and outside world.
   */
  virtual void requestInit() = 0;
  virtual void requestExit() = 0;
  virtual void analyzeInstr(Tracelet& t, NormalizedInstruction& i) = 0;
  virtual TCA funcPrologue(Func* f, int nArgs) = 0;
  virtual TCA getCallToExit() = 0;
  virtual TCA getRetFromInterpretedFrame() = 0;
  virtual void defineCns(StringData* name) = 0;
  virtual std::string getUsage() = 0;
  virtual bool dumpTC(bool ignoreLease = false) = 0;
  virtual bool dumpTCCode(const char *filename) = 0;
  virtual bool dumpTCData() = 0;
  virtual void protectCode() = 0;
  virtual void unprotectCode() = 0;
  virtual bool isValidCodeAddress(TCA) const = 0;

  /*
   * Resume is the main entry point for the translator from the
   * bytecode interpreter (see enterVMWork).  It operates on behalf of
   * a given nested invocation of the intepreter (calling back into it
   * as necessary for blocks that need to be interpreted).
   */
  virtual void resume(SrcKey sk) = 0;

  enum FuncPrologueFlags {
    FuncPrologueNormal      = 0,
    FuncPrologueMagicCall   = 1,
    FuncPrologueIntercepted = 2,
  };

  const TransDB& getTransDB() const {
    return m_transDB;
  }

  const TransRec* getTransRec(TCA tca) const {
    if (!isTransDBEnabled()) return NULL;

    TransDB::const_iterator it = m_transDB.find(tca);
    if (it == m_transDB.end()) {
      return NULL;
    }
    if (it->second >= m_translations.size()) {
      return NULL;
    }
    return &m_translations[it->second];
  }

  const TransRec* getTransRec(TransID transId) const {
    if (!isTransDBEnabled()) return NULL;

    assert(transId < m_translations.size());
    return &m_translations[transId];
  }

  TransID getNumTrans() const {
    return m_translations.size();
  }

  TransID getCurrentTransID() const {
    return m_translations.size();
  }

  uint64* getTransCounterAddr();

  uint64 getTransCounter(TransID transId) const;

  void setTransCounter(TransID transId, uint64 value);

  uint32 addTranslation(const TransRec& transRec) {
    if (!isTransDBEnabled()) return -1u;
    uint32 id = getCurrentTransID();
    m_translations.push_back(transRec);
    m_translations[id].setID(id);

    if (transRec.aLen > 0) {
      m_transDB[transRec.aStart] = id;
    }
    if (transRec.astubsLen > 0) {
      m_transDB[transRec.astubsStart] = id;
    }

    return id;
  }

  void postAnalyze(NormalizedInstruction* ni, SrcKey& sk,
                   int& currentStackOffset, Tracelet& t,
                   TraceletContext& tas);
  void analyze(const SrcKey* sk, Tracelet& out);
  void advance(Opcode const **instrs);
  static int locPhysicalOffset(Location l, const Func* f = NULL);
  static Location tvToLocation(const TypedValue* tv, const TypedValue* frame);
  static bool typeIsString(DataType type) {
    return type == KindOfString || type == KindOfStaticString;
  }
  static bool liveFrameIsPseudoMain();

  inline void sync() {
    if (tl_regState == REGSTATE_CLEAN) return;
    syncWork();
  }

  inline bool stateIsDirty() {
    return tl_regState == REGSTATE_DIRTY;
  }

  inline bool isTransDBEnabled() const {
    return debug || RuntimeOption::EvalDumpTC;
  }

  /*
   * If this returns true, we dont generate guards for any of the
   * inputs to this instruction (this is essentially to avoid
   * generating guards on behalf of interpreted instructions).
   */
  virtual bool dontGuardAnyInputs(Opcode op) { return false; }

protected:
  PCFilter m_dbgBLPC;
  SrcKeySet m_dbgBLSrcKey;
  Mutex m_dbgBlacklistLock;
  bool isSrcKeyInBL(const Unit* unit, const SrcKey& sk);

public:
  void clearDbgBL();
  bool addDbgBLPC(PC pc);
  virtual bool addDbgGuards(const Unit* unit) = 0;
  virtual bool addDbgGuard(const Func* func, Offset offset) = 0;

  TCA getResumeHelper() {
    return m_resumeHelper;
  }
};

int getStackDelta(const NormalizedInstruction& ni);

enum ControlFlowInfo {
  ControlFlowNone,
  ControlFlowChangesPC,
  ControlFlowBreaksBB
};

static inline ControlFlowInfo
opcodeControlFlowInfo(const Opcode instr) {
  switch (instr) {
    case OpJmp:
    case OpJmpZ:
    case OpJmpNZ:
    case OpSwitch:
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
    case OpEval:
    case OpNativeImpl:
    case OpContHandle:
      return ControlFlowBreaksBB;
    case OpFCall:
    case OpFCallArray:
    case OpIncl:
    case OpInclOnce:
    case OpReq:
    case OpReqOnce:
    case OpReqDoc:
    case OpReqMod:
    case OpReqSrc:
      return ControlFlowChangesPC;
    default:
      return ControlFlowNone;
  }
}

/*
 * opcodeChangesPC --
 *
 *   Returns true if the instruction can potentially set PC to point
 *   to something other than the next instruction in the bytecode
 */
static inline bool
opcodeChangesPC(const Opcode instr) {
  return opcodeControlFlowInfo(instr) >= ControlFlowChangesPC;
}

/*
 * opcodeBreaksBB --
 *
 *   Returns true if the instruction always breaks a tracelet. Most
 *   instructions that change PC will break the tracelet, though some
 *   do not (ex. FCall).
 */
static inline bool
opcodeBreaksBB(const Opcode instr) {
  return opcodeControlFlowInfo(instr) == ControlFlowBreaksBB;
}

bool outputDependsOnInput(const Opcode instr);

extern bool tc_dump();
const Func* lookupImmutableMethod(const Class* cls, const StringData* name,
                                  bool& magicCall, bool staticLookup);

bool freeLocalsInline();

} } } // HPHP::VM::Transl

#endif
