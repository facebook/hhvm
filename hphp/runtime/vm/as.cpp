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

/*
 * This module contains an assembler implementation for HHBC.  It is
 * probably fairly close to allowing you to access most of the
 * metadata associated with hhvm's compiled unit format, although it's
 * possible something has been overlooked.
 *
 * To use it, run hhvm with -v Eval.AllowHhas=true on a file with a
 * ".hhas" extension.  The syntax is probably easiest to understand by
 * looking at some examples (or the semi-BNF markup around some of the
 * parse functions here).  For examples, see hphp/tests/vm/asm_*.
 *
 *
 * Notes:
 *
 *    - You can crash hhvm very easily with this.
 *
 *      Using this module, you can emit pretty much any sort of not
 *      trivially-illegal bytecode stream, and many trivially-illegal
 *      ones as well.  You can also easily create Units with illegal
 *      metadata.  Generally this will crash the VM.  In other cases
 *      (especially if you don't bother to DefCls your classes in your
 *      .main) you'll just get mysterious "class not defined" errors
 *      or weird behavior.
 *
 *    - Whitespace is not normally significant, but newlines may not
 *      be in the middle of a list of opcode arguments.  (After the
 *      newline, the next thing seen is expected to be either a
 *      mnemonic for the next opcode in the stream or some sort of
 *      directive.)  However, newlines (and comments) may appear
 *      *inside* certain opcode arguments (e.g. string literals or
 *      vector immediates).
 *
 *      Rationale: this is partially intended to make it trivial to
 *      catch wrong-number-of-arguments errors, although it probably
 *      could be done without this if you feel like changing it.
 *
 *
 * Caveats:
 *
 *   - It might be nice if you could refer to iterators by name
 *     instead of by index.
 *
 *   - DefCls by name would be nice.
 *
 *   - Line number information can't be propagated to the various Unit
 *     structures.  (It might make sense to do this via something like
 *     a .line directive at some point.)
 *
 *   - You can't currently create non-top functions or non-hoistable
 *     classes.
 *
 *   - Missing support for static variables in a function/method.
 *
 * @author Jorden DeLong <delong.j@fb.com>
 */

#include "hphp/runtime/vm/as.h"

#include <cstdio>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>

#include "folly/String.h"

#include "hphp/util/md5.h"
#include "hphp/runtime/vm/as-shared.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/preclass-emit.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/system/systemlib.h"

TRACE_SET_MOD(hhas);

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

struct AsmState;
typedef void (*ParserFunc)(AsmState& as);

struct Error : std::runtime_error {
  explicit Error(int where, const std::string& what)
    : std::runtime_error(str(
        boost::format("Assembler Error: line %1%: %2%") % where % what))
  {}
};

struct Input {
  explicit Input(std::istream& in)
    : m_in(in)
    , m_lineNumber(1)
  {}

  int peek() { return m_in.peek(); }

  int getc() {
    int ret = m_in.get();
    if (ret == EOF) {
      io_error_if_bad();
    } else if (ret == '\n') {
      ++m_lineNumber;
    }
    return ret;
  }

  void ungetc(char c) {
    if (c == '\n') --m_lineNumber;
    m_in.putback(c);
  }

  void expect(int c) {
    if (getc() != c) {
      error(str(boost::format("expected character `%1%'") % char(c)));
    }
  }

  /*
   * Expect `c' after possible whitespace/comments.  When convenient,
   * preferable to doing skipWhitespace/expect manually to keep the
   * line number in the error prior to the whitespace skipped.
   */
  void expectWs(int c) {
    const int currentLine = m_lineNumber;
    skipWhitespace();
    if (getc() != c) {
      throw Error(currentLine,
        str(boost::format("expected character `%1%'") % char(c)));
    }
  }

  int getLineNumber() const {
    return m_lineNumber;
  }

  // Skips whitespace, then populates word with valid bareword
  // characters.  Returns true if we read any characters into word.
  bool readword(std::string& word) {
    word.clear();
    skipWhitespace();
    consumePred(is_bareword(), std::back_inserter(word));
    return !word.empty();
  }

  // Try to consume a bareword.  Skips whitespace.  If we can't
  // consume the specified word, returns false.
  bool tryConsume(const std::string& what) {
    std::string word;
    if (!readword(word)) {
      return false;
    }
    if (word != what) {
      std::for_each(word.rbegin(), word.rend(),
                    boost::bind(&Input::ungetc, this, _1));
      return false;
    }
    return true;
  }

  // C-style character escapes, no support for unicode escapes or
  // whatnot.
  template<class OutCont>
  void escapeChar(OutCont& out) {
    auto is_oct = [&] (int i) { return i >= '0' && i <= '7'; };
    auto is_hex = [&] (int i) {
      return (i >= '0' && i <= '9') ||
             (i >= 'a' && i <= 'f') ||
             (i >= 'A' && i <= 'F');
    };
    auto hex_val = [&] (int i) -> uint32_t {
      assert(is_hex(i));
      return i >= '0' && i <= '9' ? i - '0' :
             i >= 'a' && i <= 'f' ? i - 'a' + 10 : i - 'A' + 10;
    };

    auto src = getc();
    switch (src) {
    case EOF:  error("EOF in string literal");
    case 'a':  out.push_back('\a'); break;
    case 'b':  out.push_back('\b'); break;
    case 'f':  out.push_back('\f'); break;
    case 'n':  out.push_back('\n'); break;
    case 'r':  out.push_back('\r'); break;
    case 't':  out.push_back('\t'); break;
    case 'v':  out.push_back('\v'); break;
    case '\'': out.push_back('\''); break;
    case '\"': out.push_back('\"'); break;
    case '\?': out.push_back('\?'); break;
    case '\\': out.push_back('\\'); break;
    case '\n': /* ignore */         break;
    default:
      if (is_oct(src)) {
        auto val = int64_t{src} - '0';
        for (auto i = int{0}; i < 3; ++i) {
          src = getc();
          if (!is_oct(src)) { ungetc(src); break; }
          val *= 8;
          val += src - '0';
        }
        if (val > std::numeric_limits<uint8_t>::max()) {
          error("octal escape sequence overflowed");
        }
        out.push_back(static_cast<uint8_t>(val));
        return;
      }

      if (src == 'x' || src == 'X') {
        auto val = uint64_t{0};
        if (!is_hex(peek())) error("\\x used without no following hex digits");
        do {
          src = getc();
          val *= 0x10;
          val += hex_val(src);
        } while (is_hex(peek()));
        if (val > std::numeric_limits<uint8_t>::max()) {
          error("hex escape sequence overflowed");
        }
        out.push_back(static_cast<uint8_t>(val));
        return;
      }

      error("unrecognized character escape");
    }
  }

  // Reads a quoted string with typical escaping rules.  Does not skip
  // any whitespace.  Returns true if we successfully read one, or
  // false.  EOF during the string throws.
  bool readQuotedStr(std::string& str) {
    str.clear();
    if (peek() != '\"') {
      return false;
    }
    getc();

    int c;
    while ((c = getc()) != EOF) {
      switch (c) {
      case '\"': return true;
      case '\\': escapeChar(str);  break;
      default:   str.push_back(c); break;
      }
    }
    error("EOF in string literal");
    NOT_REACHED();
    return false;
  }

  /*
   * Reads a python-style longstring, or returns false if we don't
   * have one.  Does not skip any whitespace before looking for the
   * string.
   *
   * Python longstrings start with \"\"\", and can contain any bytes
   * other than \"\"\".  A '\\' character introduces C-style escapes,
   * but there's no need to escape single quote characters.
   */
  bool readLongString(std::vector<char>& buffer) {
    if (peek() != '\"') return false;
    getc();
    if (peek() != '\"') { ungetc('\"'); return false; }
    getc();
    if (peek() != '\"') { ungetc('\"');
                          ungetc('\"'); return false; }
    getc();

    int c;
    while ((c = getc()) != EOF) {
      if (c == '\\') {
        escapeChar(buffer);
        continue;
      }
      if (c == '"') {
        c = getc();
        if (c != '"') {
          buffer.push_back('"');
          ungetc(c);
          continue;
        }
        c = getc();
        if (c != '"') {
          buffer.push_back('"');
          buffer.push_back('"');
          ungetc(c);
          continue;
        }
        return true;
      }

      buffer.push_back(c);
    }
    error("EOF in \"\"\"-string literal");
    NOT_REACHED();
    return false;
  }

  // Skips whitespace (including newlines and comments).
  void skipWhitespace() {
    for (;;) {
      skipPred(boost::is_any_of(" \t\n"));
      if (peek() == '#') {
        skipPred(!boost::is_any_of("\n"));
        expect('\n');
      } else {
        break;
      }
    }
  }

  // Skip spaces and tabs, but other whitespace (such as comments or
  // newlines) stop the skip.
  void skipSpaceTab() {
    skipPred(boost::is_any_of(" \t"));
  }

  template<class Predicate>
  void skipPred(Predicate pred) {
    int c;
    while (pred(c = peek())) { getc(); }
  }

  template<class Predicate, class OutputIterator>
  void consumePred(Predicate pred, OutputIterator out) {
    int c;
    while (pred(c = peek())) { *out++ = getc(); }
  }

private:
  struct is_bareword {
    bool operator()(int i) const {
      return isalnum(i) || i == '_' || i == '.';
    }
  };

  void error(const std::string& what) {
    throw Error(getLineNumber(), what);
  }

  void io_error_if_bad() {
    if (m_in.bad()) {
      error("I/O error reading stream: " +
        folly::errnoStr(errno).toStdString());
    }
  }

private:
  std::istream& m_in;
  int m_lineNumber;
};

struct StackDepth;

struct FPIReg {
  Offset fpushOff;
  StackDepth* stackDepth;
  int fpOff;
};

/*
 * Tracks the depth of the stack in a given block of instructions.
 *
 * This structure is linked to a block of instructions (usually starting at a
 * label), and tracks the current stack depth in this block. This tracking can
 * take two forms:
 * - Absolute depth: the depth of the stack is exactly known for this block
 * - Relative depth: the depth of the stack is unknown for now. We keep track
 *   of an offset, relative to the depth of the stack at the first instruction
 *   of the block
 */
struct StackDepth {
  int currentOffset;
  int maxOffset;
  int minOffset;
  int minOffsetLine;
  boost::optional<int> baseValue;

  /*
   * During the parsing process, when a Jmp instruction is encountered, the
   * StackDepth structure for this jump becomes linked to the StackDepth
   * structure of the label (which is added to the listeners list).
   *
   * Once the absolute depth at the jump becomes known, its StackDepth
   * instance calls the setBase method of the StackDepth instance of the label.
   * The absolute depth at the label can then be inferred from the
   * absolute depth at the jump.
   */
  std::vector<std::pair<StackDepth*, int> > listeners;

  StackDepth()
    : currentOffset(0)
    , maxOffset(0)
    , minOffset(0)
  {}

  void adjust(AsmState& as, int delta);
  void addListener(AsmState& as, StackDepth* target);
  void setBase(AsmState& as, int stackDepth);

  /*
   * Sets the baseValue such as the current stack depth matches the
   * parameter.
   *
   * If the base value is already known, it may conflict with the
   * parameter of this function. In this case, an error will be raised.
   */
  void setCurrentAbsolute(AsmState& as, int stackDepth);
};

struct Label {
  Label() : bound(false) {}

  bool bound;
  Offset target;
  StackDepth stackDepth;

  /*
   * Each label source source has an Offset where the jmp should be
   * patched up is, and an Offset from which the jump delta should be
   * computed.  (The second Offset is basically to the actual
   * jump/switch/etc instruction, while the first points to the
   * immediate.)
   */
  typedef std::vector<std::pair<Offset,Offset> > SourcesVec;
  SourcesVec sources;

  /*
   * List of a parameter ids that use this label for its DV
   * initializer.
   */
  std::vector<Id> dvInits;

  /*
   * List of EHEnt's that should have m_fault bound to the Offset of
   * this label.
   */
  std::vector<size_t> ehFaults;

  /*
   * Map from exception names to the list of EHEnt's that have a catch
   * block jumping to this label for that name.
   */
  typedef std::map<std::string,std::vector<size_t> > CatchesMap;
  CatchesMap ehCatches;
};

struct AsmState : private boost::noncopyable {
  explicit AsmState(std::istream& in)
    : in(in)
    , emittedPseudoMain(false)
    , fe(0)
    , numItersSet(false)
    , currentStackDepth(&initStackDepth)
    , stackHighWater(0)
    , fdescDepth(0)
    , fdescHighWater(0)
  {
    currentStackDepth->setBase(*this, 0);
  }

  void error(const std::string& what) {
    throw Error(in.getLineNumber(), what);
  }

  void adjustStack(int delta) {
    if (currentStackDepth == nullptr) {
      // Instruction is unreachable, nothing to do here!
      return;
    }

    currentStackDepth->adjust(*this, delta);
  }

  void adjustStackHighwater(int depth) {
    stackHighWater = std::max(stackHighWater, depth);
  }

  std::string displayStackDepth() {
    std::ostringstream stack;

    if (currentStackDepth == nullptr) {
      stack << "/";
    } else if(currentStackDepth->baseValue) {
      stack << currentStackDepth->baseValue.get() +
               currentStackDepth->currentOffset;
    } else {
      stack << "?" << currentStackDepth->currentOffset;
    }

    return stack.str();
  }

  void addLabelTarget(const std::string& name) {
    Label& label = labelMap[name];
    if (label.bound) {
      error("Duplicate label " + name);
    }
    label.bound = true;
    label.target = ue->bcPos();

    StackDepth* newStack = &label.stackDepth;

    if (currentStackDepth == nullptr) {
      // Previous instruction was unreachable
      currentStackDepth = newStack;
      return;
    }

    // The stack depth at the label depends on the current depth
    currentStackDepth->addListener(*this, newStack);
    currentStackDepth = newStack;
  }

  void addLabelJump(const std::string& name, Offset immOff, Offset opcodeOff) {
    Label& label = labelMap[name];

    if (currentStackDepth == nullptr) {
      // Jump is unreachable, nothing to do here
      return;
    }

    // The stack depth at the target must be the same as the current depth
    // (whatever this may be: it may still be unknown)
    currentStackDepth->addListener(*this, &label.stackDepth);

    label.sources.push_back(std::make_pair(immOff, opcodeOff));
  }

  void enforceStackDepth(int stackDepth) {
    if (currentStackDepth == nullptr) {
      // Current instruction is unreachable, thus the constraint
      // on the stack depth will never be violated
      return;
    }

    currentStackDepth->setCurrentAbsolute(*this, stackDepth);
  }

  void enterUnreachableRegion() {
    currentStackDepth = nullptr;
  }

  void addLabelDVInit(const std::string& name, int paramId) {
    labelMap[name].dvInits.push_back(paramId);

    // Stack depth should be 0 when entering a DV init
    labelMap[name].stackDepth.setBase(*this, 0);
  }

  void addLabelEHFault(const std::string& name, size_t ehIdx) {
    labelMap[name].ehFaults.push_back(ehIdx);

    // Stack depth should be 0 when entering a fault funclet
    labelMap[name].stackDepth.setBase(*this, 0);
  }

  void addLabelEHCatch(const std::string& what,
                       const std::string& label,
                       size_t ehIdx) {
    labelMap[label].ehCatches[what].push_back(ehIdx);

    // Stack depth should be 0 when entering a catch block
    labelMap[label].stackDepth.setBase(*this, 0);
  }

  void beginFpi(Offset fpushOff) {
    if (currentStackDepth == nullptr) {
      error("beginFpi called from unreachable instruction");
    }

    fpiRegs.push_back(FPIReg());
    FPIReg& fpi = fpiRegs.back();
    fpi.fpushOff = fpushOff;
    fpi.stackDepth = currentStackDepth;
    fpi.fpOff = currentStackDepth->currentOffset + fdescDepth;
    fdescDepth += kNumActRecCells;
    fdescHighWater = std::max(fdescDepth, fdescHighWater);
  }

  void endFpi() {
    assert(!fpiRegs.empty());

    FPIEnt& ent = fe->addFPIEnt();
    FPIReg& reg = fpiRegs.back();
    ent.m_fpushOff = reg.fpushOff;
    ent.m_fcallOff = ue->bcPos();
    ent.m_fpOff = reg.fpOff;
    if (reg.stackDepth->baseValue) {
      ent.m_fpOff += reg.stackDepth->baseValue.get();
    } else {
      // base value still unknown, this will need to be updated later
      fpiToUpdate.push_back(std::make_pair(&ent, reg.stackDepth));
    }

    fpiRegs.pop_back();
    fdescDepth -= kNumActRecCells;
  }

  void finishClass() {
    assert(!fe);
    pce = 0;
  }

  void patchLabelOffsets(const Label& label) {
    for (Label::SourcesVec::const_iterator it = label.sources.begin();
        it != label.sources.end();
        ++it) {
      ue->emitInt32(label.target - it->second, it->first);
    }

    for (std::vector<Id>::const_iterator it = label.dvInits.begin();
        it != label.dvInits.end();
        ++it) {
      fe->setParamFuncletOff(*it, label.target);
    }

    for (std::vector<size_t>::const_iterator it = label.ehFaults.begin();
        it != label.ehFaults.end();
        ++it) {
      fe->ehtab()[*it].m_fault = label.target;
    }

    for (Label::CatchesMap::const_iterator it = label.ehCatches.begin();
        it != label.ehCatches.end();
        ++it) {
      Id exId = ue->mergeLitstr(makeStaticString(it->first));
      for (std::vector<size_t>::const_iterator idx_it = it->second.begin();
          idx_it != it->second.end();
          ++idx_it) {
        fe->ehtab()[*idx_it].m_catches.push_back(
          std::make_pair(exId, label.target));
      }
    }
  }

  void finishFunction() {
    for (LabelMap::const_iterator it = labelMap.begin();
        it != labelMap.end();
        ++it) {
      if (!it->second.bound) {
        error("Undefined label " + it->first);
      }
      if (it->second.target >= ue->bcPos()) {
        error("label " + it->first + " falls of the end of the function");
      }

      patchLabelOffsets(it->second);
    }

    // Patch the FPI structures
    for (auto& kv : fpiToUpdate) {
      if (!kv.second->baseValue) {
        error("created a FPI from an unreachable instruction");
      }

      kv.first->m_fpOff += kv.second->baseValue.get();
    }

    // Stack depth should be 0 at the end of a function body
    enforceStackDepth(0);

    fe->setMaxStackCells(stackHighWater + kNumActRecCells + fdescHighWater);
    fe->finish(ue->bcPos(), false);
    ue->recordFunction(fe);

    fe = 0;
    fpiRegs.clear();
    labelMap.clear();
    numItersSet = false;
    initStackDepth = StackDepth();
    initStackDepth.setBase(*this, 0);
    currentStackDepth = &initStackDepth;
    stackHighWater = 0;
    fdescDepth = 0;
    fdescHighWater = 0;
    fpiToUpdate.clear();
  }

  int getLocalId(const std::string& name) {
    if (name[0] != '$') {
      error("local variables must be prefixed with $");
    }

    const StringData* sd = makeStaticString(name.c_str() + 1);
    fe->allocVarId(sd);
    return fe->lookupVarId(sd);
  }

  int getIterId(int32_t id) {
    if (id >= fe->numIterators()) {
      error("iterator id exceeded number of iterators in the function");
    }
    return id;
  }

  UnitEmitter* ue;
  Input in;
  bool emittedPseudoMain;

  typedef std::map<std::string,ArrayData*> ADataMap;
  ADataMap adataMap;

  // When inside a class, this state is active.
  PreClassEmitter* pce;

  // When we're doing a function or method body, this state is active.
  FuncEmitter* fe;
  std::vector<FPIReg> fpiRegs;
  typedef std::map<std::string,Label> LabelMap;
  std::map<std::string,Label> labelMap;
  bool numItersSet;
  StackDepth initStackDepth;
  StackDepth* currentStackDepth;
  int stackHighWater;
  int fdescDepth;
  int fdescHighWater;
  std::vector<std::pair<FPIEnt*, StackDepth*> > fpiToUpdate;
};


void StackDepth::adjust(AsmState& as, int delta) {
  currentOffset += delta;

  if (!baseValue) {
    // The absolute stack depth is unknown. We only store the min
    // and max offsets, and we will take a decision later, when the
    // base value will be known.
    maxOffset = std::max(currentOffset, maxOffset);
    if (currentOffset < minOffset) {
      minOffsetLine = as.in.getLineNumber();
      minOffset = currentOffset;
    }
    return;
  }

  if (*baseValue + currentOffset < 0) {
    as.error("opcode sequence caused stack depth to go negative");
  }

  as.adjustStackHighwater(*baseValue + currentOffset);
}

void StackDepth::addListener(AsmState& as, StackDepth* target) {
  if (baseValue) {
    target->setBase(as, *baseValue + currentOffset);
  } else {
    listeners.push_back(std::make_pair(target, currentOffset));
  }
}

void StackDepth::setBase(AsmState& as, int stackDepth) {
  if (baseValue && stackDepth != *baseValue) {
    as.error("stack depth do not match");
  }

  baseValue = stackDepth;

  // We finally know the base value. Update AsmState accordingly.
  if (*baseValue + minOffset < 0) {
    throw Error(
      minOffsetLine,
      "opcode sequence caused stack depth to go negative"
    );
  }
  as.adjustStackHighwater(*baseValue + maxOffset);

  // Update the listeners
  for (auto& kv : listeners) {
    kv.first->setBase(as, *baseValue + kv.second);
  }

  // We won't need them anymore
  listeners.clear();
}

void StackDepth::setCurrentAbsolute(AsmState& as, int stackDepth) {
  setBase(as, stackDepth - currentOffset);
}

//////////////////////////////////////////////////////////////////////

/*
 * Opcode arguments must be on the same line as the opcode itself,
 * although certain argument types may contain internal newlines (see,
 * for example, read_immvector, read_jmpvector, or string literals).
 */
template<class Target> Target read_opcode_arg(AsmState& as) {
  as.in.skipSpaceTab();
  std::string strVal;
  as.in.consumePred(!boost::is_any_of(" \t\n#;>"),
                    std::back_inserter(strVal));
  if (strVal.empty()) {
    as.error("expected opcode or directive argument");
  }
  try {
    return boost::lexical_cast<Target>(strVal);
  } catch (boost::bad_lexical_cast&) {
    as.error("couldn't convert input argument (" + strVal + ") to "
             "proper type");
    NOT_REACHED();
  }
}

template<class SubOpType>
uint8_t read_subop(AsmState& as) {
  auto const str = read_opcode_arg<std::string>(as);
  if (auto const ty = nameToSubop<SubOpType>(str.c_str())) {
    return static_cast<uint8_t>(*ty);
  }
  as.error("unknown subop name");
  NOT_REACHED();
}

const StringData* read_litstr(AsmState& as) {
  as.in.skipSpaceTab();
  std::string strVal;
  if (!as.in.readQuotedStr(strVal)) {
    as.error("expected quoted string literal");
  }
  return makeStaticString(strVal);
}

std::vector<std::string> read_strvector(AsmState& as) {
  std::vector<std::string> ret;
  as.in.skipSpaceTab();
  as.in.expect('<');
  std::string name;
  while (as.in.skipSpaceTab(), as.in.readQuotedStr(name)) {
    ret.push_back(name);
  }
  as.in.skipSpaceTab();
  as.in.expectWs('>');
  return ret;
}

ArrayData* read_litarray(AsmState& as) {
  as.in.skipSpaceTab();
  if (as.in.getc() != '@') {
    as.error("expecting an `@foo' array literal reference");
  }
  std::string name;
  if (!as.in.readword(name)) {
    as.error("expected name of .adata literal");
  }

  AsmState::ADataMap::const_iterator it = as.adataMap.find(name);
  if (it == as.adataMap.end()) {
    as.error("unknown array data literal name " + name);
  }
  return it->second;
}

void read_immvector_immediate(AsmState& as, std::vector<uchar>& ret,
                              MemberCode mcode = InvalidMemberCode) {
  if (memberCodeImmIsLoc(mcode) || mcode == InvalidMemberCode) {
    if (as.in.getc() != '$') {
      as.error("*L member code in vector immediate must be followed by "
               "a local variable name");
    }
    std::string name;
    if (!as.in.readword(name)) {
      as.error("couldn't read name for local variable in vector immediate");
    }
    encodeIvaToVector(ret, as.getLocalId("$" + name));
  } else if (memberCodeImmIsString(mcode)) {
    encodeToVector<int32_t>(ret, as.ue->mergeLitstr(read_litstr(as)));
  } else if (memberCodeImmIsInt(mcode)) {
    encodeToVector<int64_t>(ret, read_opcode_arg<int64_t>(as));
  } else {
    as.error(std::string("don't understand immediate for member code ") +
             memberCodeString(mcode));
  }
}

std::vector<uchar> read_immvector(AsmState& as, int& stackCount) {
  std::vector<uchar> ret;

  as.in.skipSpaceTab();
  as.in.expect('<');

  std::string word;
  if (!as.in.readword(word)) {
    as.error("expected location code in immediate vector");
  }

  LocationCode lcode = parseLocationCode(word.c_str());
  if (lcode == InvalidLocationCode) {
    as.error("expected location code, saw `" + word + "'");
  }
  ret.push_back(uint8_t(lcode));
  if (word[word.size() - 1] == 'L') {
    if (as.in.getc() != ':') {
      as.error("expected `:' after location code `" + word + "'");
    }
  }
  for (int i = 0; i < numLocationCodeImms(lcode); ++i) {
    read_immvector_immediate(as, ret);
  }
  stackCount = numLocationCodeStackVals(lcode);

  // Read all the member entries.
  for (;;) {
    as.in.skipWhitespace();
    if (as.in.peek() == '>') { as.in.getc(); break; }

    if (!as.in.readword(word)) {
      as.error("expected member code in immediate vector");
    }
    MemberCode mcode = parseMemberCode(word.c_str());
    if (mcode == InvalidMemberCode) {
      as.error("unrecognized member code `" + word + "'");
    }
    ret.push_back(uint8_t(mcode));
    if (memberCodeHasImm(mcode)) {
      if (as.in.getc() != ':') {
        as.error("expected `:' after member code `" + word + "'");
      }
      read_immvector_immediate(as, ret, mcode);
    } else if (mcode != MW) {
      ++stackCount;
    }
  }

  return ret;
}

// Read in a vector of iterators the format for this vector is:
// <(TYPE) ID, (TYPE) ID, ...>
// Where TYPE := Iter | MIter | CIter
// and   ID   := Integer
std::vector<uint32_t> read_itervec(AsmState& as) {
  std::vector<uint32_t> ret;

  as.in.skipSpaceTab();
  as.in.expect('<');

  std::string word;

  for (;;) {
    as.in.expectWs('(');
    if (!as.in.readword(word)) as.error("Was expecting Iterator type.");
    if (!word.compare("Iter")) ret.push_back(KindOfIter);
    else if (!word.compare("MIter")) ret.push_back(KindOfMIter);
    else if (!word.compare("CIter")) ret.push_back(KindOfCIter);
    else as.error("Unknown iterator type `" + word + "'");
    as.in.expectWs(')');

    as.in.skipSpaceTab();

    if (!as.in.readword(word)) as.error("Was expecting iterator id.");
    uint32_t iterId = folly::to<uint32_t>(word);
    ret.push_back(iterId);

    if (!isdigit(word.back())) {
      if (word.back() == '>') break;
      if (word.back() != ',') as.error("Was expecting `,'.");
    } else {
      as.in.skipSpaceTab();
      if (as.in.peek() == '>') { as.in.getc(); break; }
      as.in.expect(',');
    }
  }

  return ret;
}

// Jump tables are lists of labels.
std::vector<std::string> read_jmpvector(AsmState& as) {
  std::vector<std::string> ret;

  as.in.skipSpaceTab();
  as.in.expect('<');

  std::string word;
  while (as.in.readword(word)) {
    ret.push_back(word);
  }
  as.in.expectWs('>');

  return ret;
}

typedef std::vector<std::pair<Id, std::string>> SSwitchJmpVector;

SSwitchJmpVector read_sswitch_jmpvector(AsmState& as) {
  SSwitchJmpVector ret;

  as.in.skipSpaceTab();
  as.in.expect('<');

  std::string defLabel;
  do {
    std::string caseStr;
    if (!as.in.readQuotedStr(caseStr)) {
      as.error("expected quoted string literal");
    }

    as.in.expect(':');

    as.in.readword(defLabel);

    ret.push_back(std::make_pair(
      as.ue->mergeLitstr(makeStaticString(caseStr)),
      defLabel
    ));

    as.in.skipWhitespace();
  } while (as.in.peek() != '-');

  as.in.expect('-');
  as.in.expect(':');
  as.in.readword(defLabel);

  // -1 stand for default case.
  ret.push_back(std::make_pair(-1, defLabel));

  as.in.expect('>');

  return ret;
}

//////////////////////////////////////////////////////////////////////

typedef std::map<std::string,ParserFunc> OpcodeParserMap;
OpcodeParserMap opcode_parsers;

#define IMM_NA
#define IMM_ONE(t) IMM_##t
#define IMM_TWO(t1, t2) IMM_##t1; IMM_##t2
#define IMM_THREE(t1, t2, t3) IMM_##t1; IMM_##t2; IMM_##t3
#define IMM_FOUR(t1, t2, t3, t4) IMM_##t1; IMM_##t2; IMM_##t3; IMM_##t4

// FCall and NewPackedArray need to know the the first imm do POP_*MANY.
#define IMM_IVA do {                            \
    int imm = read_opcode_arg<int64_t>(as);     \
    as.ue->emitIVA(imm);                        \
    if (immIVA < 0) immIVA = imm;               \
  } while (0)

#define IMM_VSA \
  std::vector<std::string> vecImm = read_strvector(as);                 \
  auto const vecImmStackValues = vecImm.size();                         \
  as.ue->emitInt32(vecImmStackValues);                                  \
  for (size_t i = 0; i < vecImmStackValues; ++i) {                      \
    as.ue->emitInt32(as.ue->mergeLitstr(String(vecImm[i]).get()));      \
  }

#define IMM_SA     as.ue->emitInt32(as.ue->mergeLitstr(read_litstr(as)))
#define IMM_I64A   as.ue->emitInt64(read_opcode_arg<int64_t>(as))
#define IMM_DA     as.ue->emitDouble(read_opcode_arg<double>(as))
#define IMM_LA     as.ue->emitIVA(as.getLocalId(  \
                     read_opcode_arg<std::string>(as)))
#define IMM_IA     as.ue->emitIVA(as.getIterId( \
                     read_opcode_arg<int32_t>(as)))
#define IMM_OA(ty) as.ue->emitByte(read_subop<ty>(as));
#define IMM_AA     as.ue->emitInt32(as.ue->mergeArray(read_litarray(as)))

/*
 * There can currently be no more than one immvector per instruction,
 * and we need access to the size of the immediate vector for
 * NUM_POP_*, so the member vector guy exposes a vecImmStackValues
 * integer.
 */
#define IMM_MA                                                        \
  int vecImmStackValues = 0;                                          \
  std::vector<uchar> vecImm = read_immvector(as, vecImmStackValues);  \
  as.ue->emitInt32(vecImm.size());                                    \
  as.ue->emitInt32(vecImmStackValues);                                \
  for (size_t i = 0; i < vecImm.size(); ++i) {                        \
    as.ue->emitByte(vecImm[i]);                                       \
  }

#define IMM_ILA do {                               \
  std::vector<uint32_t> vecImm = read_itervec(as); \
  as.ue->emitInt32(vecImm.size() / 2);             \
  for (auto& i : vecImm) {                         \
    as.ue->emitInt32(i);                           \
  }                                                \
} while (0)

#define IMM_BLA do {                                    \
  std::vector<std::string> vecImm = read_jmpvector(as); \
  as.ue->emitInt32(vecImm.size());                      \
  for (size_t i = 0; i < vecImm.size(); ++i) {          \
    labelJumps.push_back(                               \
      std::make_pair(vecImm[i], as.ue->bcPos()));       \
    as.ue->emitInt32(0); /* to be patched */            \
  }                                                     \
} while (0)

#define IMM_SLA do {                                       \
  SSwitchJmpVector vecImm = read_sswitch_jmpvector(as);    \
  as.ue->emitInt32(vecImm.size());                         \
  for (auto const& pair : vecImm) {                        \
    as.ue->emitInt32(pair.first);                          \
    labelJumps.push_back(                                  \
      std::make_pair(pair.second, as.ue->bcPos()));        \
    as.ue->emitInt32(0); /* to be patched */               \
  }                                                        \
} while(0)

#define IMM_BA do {                                                 \
  labelJumps.push_back(std::make_pair(                              \
    read_opcode_arg<std::string>(as),                               \
    as.ue->bcPos()                                                  \
  ));                                                               \
  as.ue->emitInt32(0);                                              \
} while (0)

#define NUM_PUSH_NOV 0
#define NUM_PUSH_ONE(a) 1
#define NUM_PUSH_TWO(a,b) 2
#define NUM_PUSH_THREE(a,b,c) 3
#define NUM_PUSH_INS_1(a) 1
#define NUM_PUSH_INS_2(a) 1
#define NUM_POP_NOV 0
#define NUM_POP_ONE(a) 1
#define NUM_POP_TWO(a,b) 2
#define NUM_POP_THREE(a,b,c) 3
#define NUM_POP_MMANY vecImmStackValues
#define NUM_POP_V_MMANY (1 + vecImmStackValues)
#define NUM_POP_R_MMANY (1 + vecImmStackValues)
#define NUM_POP_C_MMANY (1 + vecImmStackValues)
#define NUM_POP_FMANY immIVA /* number of arguments */
#define NUM_POP_CVMANY immIVA /* number of arguments */
#define NUM_POP_CVUMANY immIVA /* number of arguments */
#define NUM_POP_CMANY immIVA /* number of arguments */
#define NUM_POP_SMANY vecImmStackValues

#define O(name, imm, pop, push, flags)                            \
  void parse_opcode_##name(AsmState& as) {                        \
    UNUSED int64_t immIVA = -1;                                   \
    UNUSED auto const thisOpcode = Op::name;                      \
    UNUSED const Offset curOpcodeOff = as.ue->bcPos();            \
    std::vector<std::pair<std::string, Offset> > labelJumps;      \
                                                                  \
    TRACE(                                                        \
      4,                                                          \
      "%d\t[%s] %s\n",                                            \
      as.in.getLineNumber(),                                      \
      as.displayStackDepth().c_str(),                             \
      #name                                                       \
    );                                                            \
                                                                  \
    if (isFCallStar(Op##name)) {                                  \
      as.endFpi();                                                \
    }                                                             \
                                                                  \
    as.ue->emitOp(Op##name);                                      \
                                                                  \
    IMM_##imm;                                                    \
                                                                  \
    int stackDelta = NUM_PUSH_##push - NUM_POP_##pop;             \
    as.adjustStack(stackDelta);                                   \
                                                                  \
    if (isFPush(Op##name)) {                                      \
      as.beginFpi(curOpcodeOff);                                  \
    }                                                             \
                                                                  \
    for (auto& kv : labelJumps) {                                 \
      as.addLabelJump(kv.first, kv.second, curOpcodeOff);         \
    }                                                             \
                                                                  \
    /* Stack depth should be 0 after RetC or RetV. */             \
    if (thisOpcode == OpRetC || thisOpcode == OpRetV) {           \
      as.enforceStackDepth(0);                                    \
    }                                                             \
                                                                  \
    if (instrFlags(thisOpcode) & InstrFlags::TF) {                \
      as.enterUnreachableRegion();                                \
    }                                                             \
  }

OPCODES

#undef O

#undef IMM_I64A
#undef IMM_SA
#undef IMM_DA
#undef IMM_IVA
#undef IMM_LA
#undef IMM_BA
#undef IMM_BLA
#undef IMM_SLA
#undef IMM_OA
#undef IMM_MA
#undef IMM_AA
#undef IMM_VSA

#undef NUM_PUSH_NOV
#undef NUM_PUSH_ONE
#undef NUM_PUSH_TWO
#undef NUM_PUSH_THREE
#undef NUM_PUSH_POS_N
#undef NUM_PUSH_INS_1
#undef NUM_POP_NOV
#undef NUM_POP_ONE
#undef NUM_POP_TWO
#undef NUM_POP_THREE
#undef NUM_POP_POS_N
#undef NUM_POP_MMANY
#undef NUM_POP_V_MMANY
#undef NUM_POP_R_MMANY
#undef NUM_POP_C_MMANY
#undef NUM_POP_FMANY
#undef NUM_POP_CVMANY
#undef NUM_POP_CVUMANY
#undef NUM_POP_CMANY
#undef NUM_POP_SMANY

void initialize_opcode_map() {
#define O(name, imm, pop, push, flags) \
  opcode_parsers[#name] = parse_opcode_##name;
OPCODES
#undef O
}

struct Initializer {
  Initializer() { initialize_opcode_map(); }
} initializer;

//////////////////////////////////////////////////////////////////////

/*
 * long-string-literal: <string>
 *
 * `long-string-literal' is a python-style longstring.  See
 * readLongString for more details.
 */
String parse_long_string(AsmState& as) {
  as.in.skipWhitespace();

  std::vector<char> buffer;
  if (!as.in.readLongString(buffer)) {
    as.error("expected \"\"\"-string of serialized php data");
  }
  if (buffer.empty()) {
    as.error("empty php serialized data is not a valid php object");
  }

  // String wants a null, and dereferences one past the size we give
  // it.
  buffer.push_back('\0');
  return String(&buffer[0], buffer.size() - 1, CopyString);
}

/*
 * php-serialized : long-string-literal
 *                ;
 *
 * `long-string-literal' is a python-style longstring.  See
 * readLongString for more details.
 *
 * Returns a Variant representing the serialized data.  It's up to the
 * caller to make sure it is a legal literal.
 */
Variant parse_php_serialized(AsmState& as) {
  return unserialize_from_string(parse_long_string(as));
}

/*
 * directive-numiters : integer ';'
 *                    ;
 */
void parse_numiters(AsmState& as) {
  if (as.numItersSet) {
    as.error("only one .numiters directive may appear in a given function");
  }
  int32_t count = read_opcode_arg<int32_t>(as);
  as.numItersSet = true;
  as.fe->setNumIterators(count);
  as.in.expectWs(';');
}

void parse_function_body(AsmState&, int nestLevel = 0);

/*
 * directive-fault : identifier integer? '{' function-body
 *                 ;
 */
void parse_fault(AsmState& as, int nestLevel) {
  const Offset start = as.ue->bcPos();

  std::string label;
  if (!as.in.readword(label)) {
    as.error("expected label name after .try_fault");
  }
  int iterId = -1;
  as.in.skipWhitespace();
  if (as.in.peek() != '{') {
    iterId = read_opcode_arg<int32_t>(as);
  }
  as.in.expectWs('{');
  parse_function_body(as, nestLevel + 1);

  EHEnt& eh = as.fe->addEHEnt();
  eh.m_type = EHEnt::Type::Fault;
  eh.m_base = start;
  eh.m_past = as.ue->bcPos();
  eh.m_iterId = iterId;

  as.addLabelEHFault(label, as.fe->ehtab().size() - 1);
}

/*
 * directive-catch : catch-spec+ '{' function-body
 *                 ;
 *
 * catch-spec : '(' identifier identifier ')'
 *            ;
 */
void parse_catch(AsmState& as, int nestLevel) {
  const Offset start = as.ue->bcPos();

  std::vector<std::pair<std::string,std::string> > catches;
  size_t numCatches = 0;
  as.in.skipWhitespace();
  for (; as.in.peek() == '('; ++numCatches) {
    as.in.getc();

    std::string except, label;
    if (!as.in.readword(except) || !as.in.readword(label)) {
      as.error("expected (ExceptionType label) after .try_catch");
    }

    as.in.expectWs(')');

    catches.push_back(std::make_pair(except, label));
    as.in.skipWhitespace();
  }
  if (catches.empty()) {
    as.error("expected at least one (ExceptionType label) pair "
             "after .try_catch");
  }

  as.in.expect('{');
  parse_function_body(as, nestLevel + 1);

  EHEnt& eh = as.fe->addEHEnt();
  eh.m_type = EHEnt::Type::Catch;
  eh.m_base = start;
  eh.m_past = as.ue->bcPos();
  eh.m_iterId = -1;

  for (size_t i = 0; i < catches.size(); ++i) {
    as.addLabelEHCatch(catches[i].first,
                       catches[i].second,
                       as.fe->ehtab().size() - 1);
  }
}

/*
 * function-body :  fbody-line* '}'
 *               ;
 *
 * fbody-line :  ".numiters" directive-numiters
 *            |  ".try_fault" directive-fault
 *            |  ".try_catch" directive-catch
 *            |  label-name
 *            |  opcode-line
 *            ;
 *
 * label-name : identifier ':'
 *            ;
 *
 * opcode-line : opcode-mnemonic <junk that depends on opcode> '\n'
 *             ;
 */
void parse_function_body(AsmState& as, int nestLevel /* = 0 */) {
  std::string word;
  for (;;) {
    as.in.skipWhitespace();
    if (as.in.peek() == '}') {
      as.in.getc();
      if (!nestLevel) {
        as.finishFunction();
      }
      return;
    }

    if (!as.in.readword(word)) {
      as.error("unexpected directive or opcode line in function body");
    }
    if (word[0] == '.') {
      if (word == ".numiters")  { parse_numiters(as); continue; }
      if (word == ".try_fault") { parse_fault(as, nestLevel); continue; }
      if (word == ".try_catch") { parse_catch(as, nestLevel); continue; }
      as.error("unrecognized directive `" + word + "' in function");
    }
    if (as.in.peek() == ':') {
      as.in.getc();
      as.addLabelTarget(word);
      continue;
    }

    // Ok, it better be an opcode now.
    OpcodeParserMap::const_iterator it = opcode_parsers.find(word);
    if (it == opcode_parsers.end()) {
      as.error("unrecognized opcode `" + word + "'");
    }
    it->second(as);

    as.in.skipSpaceTab();
    if (as.in.peek() != '\n' && as.in.peek() != '#' && as.in.peek() != EOF) {
      as.error("too many arguments for opcode `" + word + "'");
    }
  }
}

/*
 * attribute-list : empty
 *                | '[' attribute-name* ']'
 *                ;
 *
 * The `attribute-name' rule is context-sensitive; see as-shared.cpp.
 */
Attr parse_attribute_list(AsmState& as, AttrContext ctx) {
  as.in.skipWhitespace();
  int ret = AttrNone;
  if (ctx == AttrContext::Class || ctx == AttrContext::Func) {
    if (!SystemLib::s_inited) {
      ret |= AttrUnique | AttrPersistent | AttrBuiltin;
    }
  }
  if (as.in.peek() != '[') return Attr(ret);
  as.in.getc();

  std::string word;
  for (;;) {
    as.in.skipWhitespace();
    if (as.in.peek() == ']') break;
    if (!as.in.readword(word)) break;

    auto const abit = string_to_attr(ctx, word);
    if (abit) {
      ret |= *abit;
      continue;
    }

    as.error("unrecognized attribute `" + word + "' in this context");
  }
  as.in.expect(']');
  return Attr(ret);
}

/*
 * parameter-list : '(' param-name-list ')'
 *                ;
 *
 * param-name-list : empty
 *                 | param-name ',' param-name-list
 *                 ;
 *
 * param-name : '$' identifier dv-initializer
 *            | '&' '$' identifier dv-initializer
 *            ;
 *
 * dv-initializer : empty
 *                | '=' identifier arg-default
 *                ;
 *
 * arg-default : empty
 *             | '(' long-string-literal ')'
 *             ;
 */
void parse_parameter_list(AsmState& as) {
  as.in.skipWhitespace();
  if (as.in.peek() != '(') return;
  as.in.getc();

  // Once we see one dv-initializer, every parameter after that must
  // have a dv-initializer.
  bool inDVInits = false;

  for (;;) {
    FuncEmitter::ParamInfo param;

    as.in.skipWhitespace();
    int ch = as.in.getc();
    if (ch == ')') break; // allow empty param lists
    if (ch == '.') {
      if (as.in.getc() != '.' ||
          as.in.getc() != '.') {
        as.error("expecting '...'");
      }
      as.in.expectWs(')');
      as.fe->setAttrs(as.fe->attrs() | AttrMayUseVV);
      break;
    }
    if (ch == '&') { param.setRef(true); ch = as.in.getc(); }
    if (ch != '$') {
      as.error("function parameters must have a $ prefix");
    }
    std::string name;
    if (!as.in.readword(name)) {
      as.error("expected parameter name after $");
    }

    as.in.skipWhitespace();
    ch = as.in.getc();
    if (ch == '=') {
      inDVInits = true;

      std::string label;
      if (!as.in.readword(label)) {
        as.error("expected label name for dv-initializer");
      }
      as.addLabelDVInit(label, as.fe->numParams());

      as.in.skipWhitespace();
      ch = as.in.getc();
      if (ch == '(') {
        String str = parse_long_string(as);
        param.setPhpCode(makeStaticString(str));
        TypedValue tv;
        tvWriteUninit(&tv);
        if (str.size() == 4) {
          if (!strcasecmp("null", str.data())) {
            tvWriteNull(&tv);
          } else if (!strcasecmp("true", str.data())) {
            tv = make_tv<KindOfBoolean>(true);
          }
        } else if (str.size() == 5 && !strcasecmp("false", str.data())) {
          tv = make_tv<KindOfBoolean>(false);
        }
        if (tv.m_type != KindOfUninit) {
          param.setDefaultValue(tv);
        }
        as.in.expectWs(')');
        as.in.skipWhitespace();
        ch = as.in.getc();
      }
    } else {
      if (inDVInits) {
        as.error("all parameters after the first with a dv-initializer "
                 "must have a dv-initializer");
      }
    }

    as.fe->appendParam(makeStaticString(name), param);

    if (ch == ')') break;
    if (ch != ',') as.error("expected , between parameter names");
  }
}

/*
 * directive-function : attribute-list identifier parameter-list
 *                        '{' function-body
 *                    ;
 */
void parse_function(AsmState& as) {
  if (!as.emittedPseudoMain) {
    as.error(".function blocks must all follow the .main block");
  }

  Attr attrs = parse_attribute_list(as, AttrContext::Func);
  std::string name;
  if (!as.in.readword(name)) {
    as.error(".function must have a name");
  }

  as.fe = as.ue->newFuncEmitter(makeStaticString(name));
  as.fe->init(as.in.getLineNumber(), as.in.getLineNumber() + 1 /* XXX */,
              as.ue->bcPos(), attrs, true, 0);

  parse_parameter_list(as);
  as.in.expectWs('{');

  parse_function_body(as);
}

/*
 * directive-method : attribute-list identifier parameter-list
 *                      '{' function-body
 *                  ;
 */
void parse_method(AsmState& as) {
  as.in.skipWhitespace();

  Attr attrs = parse_attribute_list(as, AttrContext::Func);
  std::string name;
  if (!as.in.readword(name)) {
    as.error(".method requires a method name");
  }

  as.fe = as.ue->newMethodEmitter(makeStaticString(name), as.pce);
  as.pce->addMethod(as.fe);
  as.fe->init(as.in.getLineNumber(), as.in.getLineNumber() + 1 /* XXX */,
              as.ue->bcPos(), attrs, true, 0);

  parse_parameter_list(as);
  as.in.expectWs('{');

  parse_function_body(as);
}

/*
 * member-tv-initializer  :  '=' php-serialized ';'
 *                        |  '=' uninit ';'
 *                        |  ';'
 *                        ;
 */
TypedValue parse_member_tv_initializer(AsmState& as) {
  as.in.skipWhitespace();

  TypedValue tvInit;
  tvWriteNull(&tvInit); // Don't confuse Variant with uninit data

  int what = as.in.getc();
  if (what == '=') {
    as.in.skipWhitespace();

    if (as.in.peek() != '\"') {
      // It might be an uninitialized property/constant.
      if (!as.in.tryConsume("uninit")) {
        as.error("Expected \"\"\" or \"uninit\" after '=' in "
                 "const/property initializer");
      }
      as.in.expectWs(';');
      tvWriteUninit(&tvInit);
      return tvInit;
    }

    tvAsVariant(&tvInit) = parse_php_serialized(as);
    if (IS_STRING_TYPE(tvInit.m_type)) {
      tvInit.m_data.pstr = makeStaticString(tvInit.m_data.pstr);
      as.ue->mergeLitstr(tvInit.m_data.pstr);
    } else if (IS_ARRAY_TYPE(tvInit.m_type)) {
      tvInit.m_data.parr = ArrayData::GetScalarArray(tvInit.m_data.parr);
      as.ue->mergeArray(tvInit.m_data.parr);
    } else if (tvInit.m_type == KindOfObject) {
      as.error("property initializer can't be an object");
    } else if (tvInit.m_type == KindOfResource) {
      as.error("property initializer can't be a resource");
    }
    as.in.expectWs(';');
  } else if (what == ';') {
    // already null
  } else {
    as.error("expected '=' or ';' after property name");
  }

  return tvInit;
}

/*
 * directive-property : attribute-list identifier member-tv-initializer
 *                    ;
 *
 */
void parse_property(AsmState& as) {
  as.in.skipWhitespace();

  Attr attrs = parse_attribute_list(as, AttrContext::Prop);
  std::string name;
  if (!as.in.readword(name)) {
    as.error("expected name for property");
  }

  TypedValue tvInit = parse_member_tv_initializer(as);
  as.pce->addProperty(makeStaticString(name),
                      attrs, empty_string.get(),
                      empty_string.get(),
                      &tvInit,
                      KindOfInvalid);
}

/*
 * directive-const : identifier member-tv-initializer
 *                 ;
 */
void parse_constant(AsmState& as) {
  as.in.skipWhitespace();

  std::string name;
  if (!as.in.readword(name)) {
    as.error("expected name for constant");
  }

  TypedValue tvInit = parse_member_tv_initializer(as);
  as.pce->addConstant(makeStaticString(name),
                      empty_string.get(), &tvInit,
                      empty_string.get());
}

/*
 * directive-default-ctor : ';'
 *                        ;
 *
 * Creates an 86ctor stub for the class.
 */
void parse_default_ctor(AsmState& as) {
  assert(!as.fe && as.pce);

  as.fe = as.ue->newMethodEmitter(
    makeStaticString("86ctor"), as.pce);
  as.pce->addMethod(as.fe);
  as.fe->init(as.in.getLineNumber(), as.in.getLineNumber(),
              as.ue->bcPos(), AttrPublic, true, 0);
  as.ue->emitOp(OpNull);
  as.ue->emitOp(OpRetC);
  as.stackHighWater = 1;
  as.finishFunction();

  as.in.expectWs(';');
}

/*
 * directive-use :  identifier+ ';'
 *               |  identifier+ '{' use-line* '}'
 *               ;
 *
 * use-line : use-name-ref "insteadof" identifier+ ';'
 *          | use-name-ref "as" attribute-list identifier ';'
 *          | use-name-ref "as" attribute-list ';'
 *          ;
 */
void parse_use(AsmState& as) {
  std::vector<std::string> usedTraits;
  for (;;) {
    std::string name;
    if (!as.in.readword(name)) break;
    usedTraits.push_back(name);
  }
  if (usedTraits.empty()) {
    as.error(".use requires a trait name");
  }

  for (size_t i = 0; i < usedTraits.size(); ++i) {
    as.pce->addUsedTrait(makeStaticString(usedTraits[i]));
  }
  as.in.skipWhitespace();
  if (as.in.peek() != '{') {
    as.in.expect(';');
    return;
  }
  as.in.getc();

  for (;;) {
    as.in.skipWhitespace();
    if (as.in.peek() == '}') break;

    std::string traitName;
    std::string identifier;
    if (!as.in.readword(traitName)) {
      as.error("expected identifier for line in .use block");
    }
    as.in.skipWhitespace();
    if (as.in.peek() == ':') {
      as.in.getc();
      as.in.expect(':');
      if (!as.in.readword(identifier)) {
        as.error("expected identifier after ::");
      }
    } else {
      identifier = traitName;
      traitName.clear();
    }

    if (as.in.tryConsume("as")) {
      Attr attrs = parse_attribute_list(as, AttrContext::TraitImport);
      std::string alias;
      if (!as.in.readword(alias)) {
        if (attrs != AttrNone) {
          alias = identifier;
        } else {
          as.error("expected identifier or attribute list after "
                   "`as' in .use block");
        }
      }

      as.pce->addTraitAliasRule(PreClass::TraitAliasRule(
        makeStaticString(traitName),
        makeStaticString(identifier),
        makeStaticString(alias),
        attrs));
    } else if (as.in.tryConsume("insteadof")) {
      if (traitName.empty()) {
        as.error("Must specify TraitName::name when using a trait insteadof");
      }

      PreClass::TraitPrecRule precRule(
        makeStaticString(traitName),
        makeStaticString(identifier));

      bool addedOtherTraits = false;
      std::string whom;
      while (as.in.readword(whom)) {
        precRule.addOtherTraitName(makeStaticString(whom));
        addedOtherTraits = true;
      }
      if (!addedOtherTraits) {
        as.error("one or more trait names expected after `insteadof'");
      }

      as.pce->addTraitPrecRule(precRule);
    } else {
      as.error("expected `as' or `insteadof' in .use block");
    }

    as.in.expectWs(';');
  }

  as.in.expect('}');
}

/*
 * class-body : class-body-line* '}'
 *            ;
 *
 * class-body-line : ".method"       directive-method
 *                 | ".property"     directive-property
 *                 | ".const"        directive-const
 *                 | ".use"          directive-use
 *                 | ".default_ctor" directive-default-ctor
 *                 ;
 */
void parse_class_body(AsmState& as) {
  if (!as.emittedPseudoMain) {
    as.error(".class blocks must all follow the .main block");
  }

  std::string directive;
  while (as.in.readword(directive)) {
    if (directive == ".method")       { parse_method(as);       continue; }
    if (directive == ".property")     { parse_property(as);     continue; }
    if (directive == ".const")        { parse_constant(as);     continue; }
    if (directive == ".use")          { parse_use(as);          continue; }
    if (directive == ".default_ctor") { parse_default_ctor(as); continue; }

    as.error("unrecognized directive `" + directive + "' in class");
  }
  as.in.expect('}');
  as.finishClass();
}

/*
 * directive-class : attribute-list identifier extension-clause
 *                      implements-clause '{' class-body
 *                 ;
 *
 * extension-clause : empty
 *                  | "extends" identifier
 *                  ;
 *
 * implements-clause : empty
 *                   | "implements" '(' identifier* ')'
 *                   ;
 *
 */
void parse_class(AsmState& as) {
  as.in.skipWhitespace();

  Attr attrs = parse_attribute_list(as, AttrContext::Class);
  std::string name;
  if (!as.in.readword(name)) {
    as.error(".class must have a name");
  }

  std::string parentName;
  if (as.in.tryConsume("extends")) {
    if (!as.in.readword(parentName)) {
      as.error("expected parent class name after `extends'");
    }
  }

  std::vector<std::string> ifaces;
  if (as.in.tryConsume("implements")) {
    as.in.expectWs('(');
    std::string word;
    while (as.in.readword(word)) {
      ifaces.push_back(word);
    }
    as.in.expect(')');
  }

  as.pce = as.ue->newPreClassEmitter(makeStaticString(name),
                                     PreClass::MaybeHoistable);
  as.pce->init(as.in.getLineNumber(),
               as.in.getLineNumber() + 1, // XXX
               as.ue->bcPos(),
               attrs,
               makeStaticString(parentName),
               empty_string.get());
  for (size_t i = 0; i < ifaces.size(); ++i) {
    as.pce->addInterface(makeStaticString(ifaces[i]));
  }

  as.in.expectWs('{');
  parse_class_body(as);
}

/*
 * directive-filepath : quoted-string-literal ';'
 *                    ;
 */
void parse_filepath(AsmState& as) {
  auto const str = read_litstr(as);
  as.ue->setFilepath(str);
  as.in.expectWs(';');
}

/*
 * directive-main : '{' function-body
 *                ;
 */
void parse_main(AsmState& as) {
  if (as.emittedPseudoMain) {
    if (!SystemLib::s_inited) {
      as.error(".main found in systemlib");
    } else {
      as.error("Multiple .main directives found");
    }
  }
  as.in.expectWs('{');

  as.ue->initMain(as.in.getLineNumber(),
                  as.in.getLineNumber() + 1 /* XXX */);
  as.fe = as.ue->getMain();
  as.emittedPseudoMain = true;
  parse_function_body(as);
}

/*
 * directive-adata :  identifier '=' php-serialized ';'
 *                 ;
 */
void parse_adata(AsmState& as) {
  as.in.skipWhitespace();
  std::string dataLabel;
  if (!as.in.readword(dataLabel)) {
    as.error("expected name for .adata");
  }
  if (as.adataMap.count(dataLabel)) {
    as.error("duplicate adata label name " + dataLabel);
  }

  as.in.expectWs('=');
  Variant var = parse_php_serialized(as);
  if (!var.isArray()) {
    as.error(".adata only supports serialized arrays");
  }
  Array arr(var.toArray());
  ArrayData* data = ArrayData::GetScalarArray(arr.get());
  as.ue->mergeArray(data);
  as.adataMap[dataLabel] = data;

  as.in.expectWs(';');
}

/*
 * asm-file : asm-tld* <EOF>
 *          ;
 *
 * asm-tld :    ".filepath"    directive-filepath
 *         |    ".main"        directive-main
 *         |    ".function"    directive-function
 *         |    ".adata"       directive-adata
 *         |    ".class"       directive-class
 *         ;
 */
void parse(AsmState& as) {
  as.in.skipWhitespace();
  std::string directive;
  if (!SystemLib::s_inited) {
    /*
     * The SystemLib::s_hhas_unit is required to be merge-only,
     * and we create the source by concatenating separate .hhas files
     * Rather than choosing one to have the .main directive, we just
     * generate a trivial pseudoMain automatically.
     */
    as.ue->addTrivialPseudoMain();
    as.emittedPseudoMain = true;
  }

  while (as.in.readword(directive)) {
    if (directive == ".filepath")    { parse_filepath(as); continue; }
    if (directive == ".main")        { parse_main(as);     continue; }
    if (directive == ".function")    { parse_function(as); continue; }
    if (directive == ".adata")       { parse_adata(as);    continue; }
    if (directive == ".class")       { parse_class(as);    continue; }

    as.error("unrecognized top-level directive `" + directive + "'");
  }

  if (!as.emittedPseudoMain) {
    as.error("no .main found in hhas unit");
  }
}

}

//////////////////////////////////////////////////////////////////////

UnitEmitter* assemble_string(const char* code, int codeLen,
                             const char* filename, const MD5& md5) {
  std::unique_ptr<UnitEmitter> ue(new UnitEmitter(md5));
  StringData* sd = makeStaticString(filename);
  ue->setFilepath(sd);

  try {
    std::istringstream instr(std::string(code, codeLen));
    AsmState as(instr);
    as.ue = ue.get();
    parse(as);
  } catch (const std::exception& e) {
    ue.reset(new UnitEmitter(md5));
    ue->setFilepath(sd);
    ue->initMain(1, 1);
    ue->emitOp(OpString);
    ue->emitInt32(ue->mergeLitstr(makeStaticString(e.what())));
    ue->emitOp(OpFatal);
    ue->emitByte(static_cast<uint8_t>(FatalOp::Runtime));
    FuncEmitter* fe = ue->getMain();
    fe->setMaxStackCells(kNumActRecCells + 1);
    // XXX line numbers are bogus
    fe->finish(ue->bcPos(), false);
    ue->recordFunction(fe);
  }

  return ue.release();
}

//////////////////////////////////////////////////////////////////////

}
