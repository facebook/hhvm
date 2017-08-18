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
 * Wishlist:
 *
 *   - It might be nice if you could refer to iterators by name
 *     instead of by index.
 *
 *   - DefCls by name would be nice.
 *
 * Missing features (partial list):
 *
 *   - builtinType (for native funcs) field on ParamInfo
 *
 *   - while class/function names can contains ':', '$', and ';',
 *     .use declarations can't handle those names because of syntax
 *     conflicts
 *
 * @author Jordan DeLong <delong.j@fb.com>
 */

#include "hphp/runtime/vm/as.h"

#include <cstdio>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>

#include <folly/Conv.h>
#include <folly/MapUtil.h>
#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/String.h>

#include "hphp/util/md5.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/vm/as-shared.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/system/systemlib.h"

TRACE_SET_MOD(hhas);

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

struct AsmState;
typedef void (*ParserFunc)(AsmState& as);

struct Error : std::runtime_error {
  explicit Error(int where, const std::string& what)
    : std::runtime_error(folly::sformat(
        "Assembler Error: line {}: {}", where, what))
  {}
};

struct Input {
  explicit Input(std::istream& in)
    : m_in(in)
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
      error(folly::sformat("expected character `{}'", char(c)));
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
        folly::sformat("expected character `{}'", char(c)));
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
  // Skips whitespace, then populates name with valid extname
  // characters.  Returns true if we read any characters into name.
  bool readname(std::string& name) {
    name.clear();
    skipWhitespace();
    consumePred(is_extname(), std::back_inserter(name));
    return !name.empty();
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
  int32_t readint() {
    std::string buf;
    skipWhitespace();
    if (peek() == '-') buf += (char)getc();
    consumePred(isdigit, std::back_inserter(buf));
    if (buf.empty() || buf == "-") {
      throw Error(m_lineNumber, "expected integral value");
    }
    return folly::to<int32_t>(buf);
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
    case '\r': /* ignore */         break;
    case '\n': /* ignore */         break;
    default:
      if (is_oct(src)) {
        auto val = int64_t{src} - '0';
        for (auto i = int{1}; i < 3; ++i) {
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
        for (auto i = int{0}; i < 2; ++i) {
          src = getc();
          if (!is_hex(src)) { ungetc(src); break; }
          val *= 0x10;
          val += hex_val(src);
        }
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
    not_reached();
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
    not_reached();
    return false;
  }

  // Skips whitespace (including newlines and comments).
  void skipWhitespace() {
    for (;;) {
      skipPred(boost::is_any_of(" \t\r\n"));
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
      return isalnum(i) || i == '_' || i == '.' || i == '$' || i == '\\';
    }
  };
  // whether a character is a valid part of the extended sorts of
  // names that HHVM uses for certain generated constructs
  // (closures, __Memoize implementations, etc)
  struct is_extname {
    bool operator()(int i) const {
      is_bareword is_bw;
      return is_bw(i) || i == ':' || i == ';' || i == '#' || i =='@' ||
        (i >= 0x7f && i <= 0xff) /* see hphp.ll :( */;
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
  int m_lineNumber{1};
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
  /*
   * Tracks the max depth of elem stack + desc stack offset inside a region
   * where baseValue is unknown.
   */
  int maxOffset;
  /*
   * Tracks the min depth of the elem stack inside a region where baseValue
   * is unknown, and the line where the min occurred.
   */
  int minOffset;
  int minOffsetLine;
  folly::Optional<int> baseValue;

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
  int absoluteDepth() {
    assert(baseValue.hasValue());
    return baseValue.value() + currentOffset;
  }

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
  bool bound{false};
  Offset target;
  StackDepth stackDepth;

  /*
   * Each label source source has an Offset where the jmp should be
   * patched up is, and an Offset from which the jump delta should be
   * computed.  (The second Offset is basically to the actual
   * jump/switch/etc instruction, while the first points to the
   * immediate.)
   */
  std::vector<std::pair<Offset,Offset>> sources;

  /*
   * List of a parameter ids that use this label for its DV
   * initializer.
   */
  std::vector<Id> dvInits;

  /*
   * List of EHEnts that have m_handler pointing to this label.
   */
  std::vector<size_t> ehEnts;
};

struct AsmState {
  explicit AsmState(std::istream& in)
    : in(in)
  {
    currentStackDepth->setBase(*this, 0);
  }

  AsmState(const AsmState&) = delete;
  AsmState& operator=(const AsmState&) = delete;

  template<typename... Args>
  void error(const std::string& fmt, Args&&... args) {
    throw Error(in.getLineNumber(),
                folly::sformat(fmt, std::forward<Args>(args)...));
  }


  void adjustStack(int delta) {
    if (currentStackDepth == nullptr) {
      // Instruction is unreachable, nothing to do here!
      return;
    }

    currentStackDepth->adjust(*this, delta);
  }

  void adjustStackHighwater(int depth) {
    if (depth) {
      fe->maxStackCells = std::max(fe->maxStackCells, depth);
    }
  }

  std::string displayStackDepth() {
    std::ostringstream stack;

    if (currentStackDepth == nullptr) {
      stack << "/";
    } else if (currentStackDepth->baseValue) {
      stack << *currentStackDepth->baseValue +
               currentStackDepth->currentOffset;
    } else {
      stack << "?" << currentStackDepth->currentOffset;
    }

    return stack.str();
  }

  void addLabelTarget(const std::string& name) {
    auto& label = labelMap[name];
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
    auto& label = labelMap[name];

    if (currentStackDepth != nullptr) {
      // The stack depth at the target must be the same as the current depth
      // (whatever this may be: it may still be unknown)
      currentStackDepth->addListener(*this, &label.stackDepth);
    }

    label.sources.emplace_back(immOff, opcodeOff);
  }

  void enforceStackDepth(int stackDepth) {
    if (currentStackDepth == nullptr) {
      // Current instruction is unreachable, thus the constraint
      // on the stack depth will never be violated
      return;
    }

    currentStackDepth->setCurrentAbsolute(*this, stackDepth);
  }

  bool isUnreachable() {
    return currentStackDepth == nullptr;
  }

  void enterUnreachableRegion() {
    currentStackDepth = nullptr;
  }

  void enterReachableRegion(int stackDepth) {
    unnamedStackDepths.emplace_back(std::make_unique<StackDepth>());
    currentStackDepth = unnamedStackDepths.back().get();
    currentStackDepth->setBase(*this, stackDepth);
  }

  void addLabelDVInit(const std::string& name, int paramId) {
    labelMap[name].dvInits.push_back(paramId);

    // Stack depth should be 0 when entering a DV init
    labelMap[name].stackDepth.setBase(*this, 0);
  }

  void addLabelEHEnt(const std::string& name, size_t ehIdx) {
    labelMap[name].ehEnts.push_back(ehIdx);

    // Stack depth should be 0 when entering a fault funclet
    labelMap[name].stackDepth.setBase(*this, 0);
  }

  void beginFpi(Offset fpushOff) {
    fpiRegs.push_back(FPIReg{
      fpushOff,
      currentStackDepth,
      currentStackDepth->currentOffset
    });
    fdescDepth += kNumActRecCells;
    currentStackDepth->adjust(*this, 0);
  }

  void endFpi() {
    if (fpiRegs.empty()) {
      error("endFpi called with no active fpi region");
    }

    auto& ent = fe->addFPIEnt();
    const auto& reg = fpiRegs.back();
    ent.m_fpushOff = reg.fpushOff;
    ent.m_fpiEndOff = ue->bcPos();
    ent.m_fpOff = reg.fpOff;
    if (reg.stackDepth->baseValue) {
      ent.m_fpOff += *reg.stackDepth->baseValue;
    } else {
      // Base value still unknown, this will need to be updated later.

      // Store the FPIEnt's index in the FuncEmitter's entry table.
      assert(&fe->fpitab[fe->fpitab.size()-1] == &ent);
      fpiToUpdate.emplace_back(fe->fpitab.size() - 1, reg.stackDepth);
    }

    fpiRegs.pop_back();
    always_assert(fdescDepth >= kNumActRecCells);
    fdescDepth -= kNumActRecCells;
  }

  void finishClass() {
    assert(!fe);
    ue->addPreClassEmitter(pce);
    pce = 0;
    enumTySet = false;
  }

  void patchLabelOffsets(const Label& label) {
    for (auto const& source : label.sources) {
      ue->emitInt32(label.target - source.second, source.first);
    }

    for (auto const& dvinit : label.dvInits) {
      fe->params[dvinit].funcletOff = label.target;
    }

    for (auto const& ehEnt : label.ehEnts) {
      fe->ehtab[ehEnt].m_handler = label.target;
    }
  }

  void finishSection() {
    for (auto const& label : labelMap) {
      if (!label.second.bound) {
        error("Undefined label " + label.first);
      }
      if (label.second.target >= ue->bcPos()) {
        error("label " + label.first + " falls of the end of the function");
      }

      patchLabelOffsets(label.second);
    }

    // Patch the FPI structures
    for (auto& kv : fpiToUpdate) {
      if (!kv.second->baseValue) {
        error("created a FPI from an unreachable instruction");
      }

      fe->fpitab[kv.first].m_fpOff += *kv.second->baseValue;
    }
  }

  void finishFunction() {
    finishSection();

    // Stack depth should be 0 at the end of a function body
    enforceStackDepth(0);

    // Bump up the unnamed local count
    const int numLocals = maxUnnamed + 1;
    while (fe->numLocals() < numLocals) {
      fe->allocUnnamedLocal();
    }

    fe->maxStackCells +=
      fe->numLocals() +
      fe->numIterators() * kNumIterCells +
      clsRefCountToCells(fe->numClsRefSlots());

    fe->finish(ue->bcPos(), false);
    ue->recordFunction(fe);

    fe = 0;
    fpiRegs.clear();
    labelMap.clear();
    numItersSet = false;
    numClsRefSlotsSet = false;
    initStackDepth = StackDepth();
    initStackDepth.setBase(*this, 0);
    currentStackDepth = &initStackDepth;
    unnamedStackDepths.clear();
    fdescDepth = 0;
    maxUnnamed = -1;
    fpiToUpdate.clear();
  }

  void resolveDynCallWrappers() {
    auto const& allFuncs = ue->fevec();
    for (auto const& p : dynCallWrappers) {
      auto const iter = std::find_if(
        allFuncs.begin(),
        allFuncs.end(),
        [&](const FuncEmitter* f){ return f->name->isame(p.second); }
      );
      if (iter == allFuncs.end()) {
        error("{} specifies unknown function {} as a dyncall wrapper",
              p.first->name, p.second);
      }
      p.first->dynCallWrapperId = (*iter)->id();
    }
    dynCallWrappers.clear();
  }

  int getLocalId(const std::string& name) {
    if (name[0] == '_') {
      int id = folly::to<int>(name.substr(1));
      if (id > maxUnnamed) maxUnnamed = id;
      return id;
    }

    if (name[0] != '$') {
      error("local variables must be prefixed with $ or _");
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

  int getClsRefSlot(int32_t slot) {
    if (slot >= fe->numClsRefSlots()) {
      error("class-ref slot id exceeded number of class-ref "
            "slots in the function");
    }
    return slot;
  }

  UnitEmitter* ue;
  Input in;
  bool emittedPseudoMain{false};
  bool emittedTopLevelFunc{false};

  std::map<std::string,ArrayData*> adataMap;
  std::map<FuncEmitter*,const StringData*> dynCallWrappers;

  // When inside a class, this state is active.
  PreClassEmitter* pce;

  // When we're doing a function or method body, this state is active.
  FuncEmitter* fe{nullptr};
  std::vector<FPIReg> fpiRegs;
  std::map<std::string,Label> labelMap;
  bool numItersSet{false};
  bool numClsRefSlotsSet{false};
  bool enumTySet{false};
  StackDepth initStackDepth;
  StackDepth* currentStackDepth{&initStackDepth};
  std::vector<std::unique_ptr<StackDepth>> unnamedStackDepths;
  int fdescDepth{0};
  int minStackDepth{0};
  int maxUnnamed{-1};
  std::vector<std::pair<size_t, StackDepth*>> fpiToUpdate;
  std::set<std::string,stdltistr> hoistables;
  std::unordered_map<uint32_t,Offset> defClsOffsets;
  Location::Range srcLoc{-1,-1,-1,-1};
};

void StackDepth::adjust(AsmState& as, int delta) {
  currentOffset += delta;

  if (!baseValue) {
    // The absolute stack depth is unknown. We only store the min
    // and max offsets, and we will take a decision later, when the
    // base value will be known.
    maxOffset = std::max(currentOffset + as.fdescDepth, maxOffset);
    if (currentOffset < minOffset) {
      minOffsetLine = as.in.getLineNumber();
      minOffset = currentOffset;
    }
    return;
  }

  if (*baseValue + currentOffset < 0) {
    as.error("opcode sequence caused stack depth to go negative");
  }

  as.adjustStackHighwater(*baseValue + currentOffset + as.fdescDepth);
}

void StackDepth::addListener(AsmState& as, StackDepth* target) {
  if (baseValue) {
    target->setBase(as, *baseValue + currentOffset);
  } else {
    listeners.emplace_back(target, currentOffset);
  }
}

void StackDepth::setBase(AsmState& as, int stackDepth) {
  if (baseValue && stackDepth != *baseValue) {
    as.error("stack depth {} does not match base value {}",
             stackDepth, *baseValue);
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
  auto l = std::move(listeners);
  // We won't need them anymore
  listeners.clear();
  for (auto& kv : l) {
    kv.first->setBase(as, *baseValue + kv.second);
  }
}

void StackDepth::setCurrentAbsolute(AsmState& as, int stackDepth) {
  setBase(as, stackDepth - currentOffset);
}

//////////////////////////////////////////////////////////////////////

/*
 * Opcode arguments must be on the same line as the opcode itself,
 * although certain argument types may contain internal newlines (see,
 * for example, read_jmpvector or string literals).
 */
template<class Target> Target read_opcode_arg(AsmState& as) {
  as.in.skipSpaceTab();
  std::string strVal;
  as.in.consumePred(!boost::is_any_of(" \t\r\n#;>"),
                    std::back_inserter(strVal));
  if (strVal.empty()) {
    as.error("expected opcode or directive argument");
  }
  try {
    return folly::to<Target>(strVal);
  } catch (std::range_error&) {
    as.error("couldn't convert input argument (" + strVal + ") to "
             "proper type");
    not_reached();
  }
}

template<class SubOpType>
uint8_t read_subop(AsmState& as) {
  auto const str = read_opcode_arg<std::string>(as);
  if (auto const ty = nameToSubop<SubOpType>(str.c_str())) {
    return static_cast<uint8_t>(*ty);
  }
  as.error("unknown subop name");
  not_reached();
}

const StringData* read_litstr(AsmState& as) {
  as.in.skipSpaceTab();
  std::string strVal;
  if (!as.in.readQuotedStr(strVal)) {
    as.error("expected quoted string literal");
  }
  return makeStaticString(strVal);
}

/*
 * maybe-string-literal : N
 *                      | string-literal
 *                      ;
 */
const StringData* read_maybe_litstr(AsmState& as) {
  as.in.skipSpaceTab();
  if (as.in.peek() == 'N') {
    as.in.getc();
    return nullptr;
  }
  return read_litstr(as);
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

  auto const it = as.adataMap.find(name);
  if (it == as.adataMap.end()) {
    as.error("unknown array data literal name " + name);
  }
  return it->second;
}

RepoAuthType read_repo_auth_type(AsmState& as) {
  auto const str = read_opcode_arg<std::string>(as);
  folly::StringPiece parse(str);

  /*
   * Note: no support for reading array types.  (The assembler only
   * emits a single unit, so it can't really be involved in creating a
   * ArrayTypeTable.)
   */

  using T = RepoAuthType::Tag;

#define X(what, tag) \
  if (parse.startsWith(what)) return RepoAuthType{tag}

#define Y(what, tag)                                  \
  if (parse.startsWith(what)) {                       \
    parse.removePrefix(what);                         \
    auto const cls = makeStaticString(parse.data());  \
    as.ue->mergeLitstr(cls);                          \
    return RepoAuthType{tag, cls};                    \
  }

  Y("Obj=",     T::ExactObj);
  Y("?Obj=",    T::OptExactObj);
  Y("?Obj<=",   T::OptSubObj);
  Y("Obj<=",    T::SubObj);

  X("Arr",      T::Arr);
  X("?Arr",     T::OptArr);
  X("Vec",      T::Vec);
  X("?Vec",     T::OptVec);
  X("Dict",     T::Dict);
  X("?Dict",    T::OptDict);
  X("Keyset",   T::Keyset);
  X("?Keyset",  T::OptKeyset);
  X("Bool",     T::Bool);
  X("?Bool",    T::OptBool);
  X("Cell",     T::Cell);
  X("Dbl",      T::Dbl);
  X("?Dbl",     T::OptDbl);
  X("Gen",      T::Gen);
  X("InitCell", T::InitCell);
  X("InitGen",  T::InitGen);
  X("InitNull", T::InitNull);
  X("InitUnc",  T::InitUnc);
  X("Int",      T::Int);
  X("?Int",     T::OptInt);
  X("Null",     T::Null);
  X("Obj",      T::Obj);
  X("?Obj",     T::OptObj);
  X("Ref",      T::Ref);
  X("?Res",     T::OptRes);
  X("Res",      T::Res);
  X("?SArr",    T::OptSArr);
  X("SArr",     T::SArr);
  X("?SVec",    T::OptSVec);
  X("SVec",     T::SVec);
  X("?SDict",   T::OptSDict);
  X("SDict",    T::SDict);
  X("?SKeyset", T::OptSKeyset);
  X("SKeyset",  T::SKeyset);
  X("?SStr",    T::OptSStr);
  X("SStr",     T::SStr);
  X("?Str",     T::OptStr);
  X("Str",      T::Str);
  X("Unc",      T::Unc);
  X("?UncArrKey", T::OptUncArrKey);
  X("?ArrKey",  T::OptArrKey);
  X("UncArrKey",T::UncArrKey);
  X("ArrKey",   T::ArrKey);
  X("Uninit",   T::Uninit);

#undef X
#undef Y

  // Make sure the above parsing code is revisited when new tags are
  // added (we'll get a warning for a missing case label):
  if (debug) switch (RepoAuthType{}.tag()) {
  case T::Uninit:
  case T::InitNull:
  case T::Null:
  case T::Int:
  case T::OptInt:
  case T::Dbl:
  case T::OptDbl:
  case T::Res:
  case T::OptRes:
  case T::Bool:
  case T::OptBool:
  case T::SStr:
  case T::OptSStr:
  case T::Str:
  case T::OptStr:
  case T::SArr:
  case T::OptSArr:
  case T::Arr:
  case T::OptArr:
  case T::SVec:
  case T::OptSVec:
  case T::Vec:
  case T::OptVec:
  case T::SDict:
  case T::OptSDict:
  case T::Dict:
  case T::OptDict:
  case T::SKeyset:
  case T::OptSKeyset:
  case T::Keyset:
  case T::OptKeyset:
  case T::Obj:
  case T::OptObj:
  case T::InitUnc:
  case T::Unc:
  case T::OptUncArrKey:
  case T::OptArrKey:
  case T::UncArrKey:
  case T::ArrKey:
  case T::InitCell:
  case T::Cell:
  case T::Ref:
  case T::InitGen:
  case T::Gen:
  case T::ExactObj:
  case T::SubObj:
  case T::OptExactObj:
  case T::OptSubObj:
    break;
  }

  as.error("unrecognized RepoAuthType format");
  not_reached();
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
    ret.push_back(folly::to<uint32_t>(word));

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

    ret.emplace_back(
      as.ue->mergeLitstr(makeStaticString(caseStr)),
      defLabel
    );

    as.in.skipWhitespace();
  } while (as.in.peek() != '-');

  as.in.expect('-');
  as.in.expect(':');
  as.in.readword(defLabel);

  // -1 stand for default case.
  ret.emplace_back(-1, defLabel);

  as.in.expect('>');

  return ret;
}

MemberKey read_member_key(AsmState& as) {
  as.in.skipWhitespace();

  std::string word;
  if (!as.in.readword(word)) as.error("expected member code");

  auto optMcode = parseMemberCode(word.c_str());
  if (!optMcode) as.error("unrecognized member code `" + word + "'");

  auto const mcode = *optMcode;
  if (mcode != MW && as.in.getc() != ':') {
    as.error("expected `:' after member code `" + word + "'");
  }

  switch (mcode) {
    case MW:
      return MemberKey{};
    case MEL: case MPL: {
      std::string name;
      if (!as.in.readword(name)) {
        as.error("couldn't read name for local variable in member key");
      }
      return MemberKey{mcode, as.getLocalId(name)};
    }
    case MEC: case MPC:
      return MemberKey{mcode, read_opcode_arg<int32_t>(as)};
    case MEI:
      return MemberKey{mcode, read_opcode_arg<int64_t>(as)};
    case MET: case MPT: case MQT:
      return MemberKey{mcode, read_litstr(as)};
  }
  not_reached();
}

LocalRange read_local_range(AsmState& as) {
  auto first = read_opcode_arg<std::string>(as);
  if (first.size() > 2 && first[0] == 'L' && first[1] == ':') {
    first = "_" + first.substr(2);
  }
  auto const pos = first.find('+');
  if (pos == std::string::npos) as.error("expecting `+' in local range");
  auto const rest = first.substr(pos + 1);
  first = first.substr(0, pos);
  auto const firstLoc = as.getLocalId(first);
  auto const restCount = folly::to<uint32_t>(rest);
  if (firstLoc + restCount > as.maxUnnamed) {
    as.maxUnnamed = firstLoc + restCount;
  }
  return LocalRange{uint32_t(firstLoc), restCount};
}

//////////////////////////////////////////////////////////////////////

std::map<std::string,ParserFunc> opcode_parsers;

#define IMM_NA
#define IMM_ONE(t) IMM_##t
#define IMM_TWO(t1, t2) IMM_ONE(t1); ++immIdx; IMM_##t2
#define IMM_THREE(t1, t2, t3) IMM_TWO(t1, t2); ++immIdx; IMM_##t3
#define IMM_FOUR(t1, t2, t3, t4) IMM_THREE(t1, t2, t3); ++immIdx; IMM_##t4

// Some bytecodes need to know an iva imm for (PUSH|POP)_*.
#define IMM_IVA do {                                      \
    auto imm = read_opcode_arg<uint32_t>(as);             \
    as.ue->emitIVA(imm);                                  \
    immIVA[immIdx] = imm;                                 \
  } while (0)

#define IMM_VSA \
  std::vector<std::string> vecImm = read_strvector(as);                 \
  auto const vecImmStackValues = vecImm.size();                         \
  as.ue->emitInt32(vecImmStackValues);                                  \
  for (size_t i = 0; i < vecImmStackValues; ++i) {                      \
    as.ue->emitInt32(as.ue->mergeLitstr(String(vecImm[i]).get()));      \
  }

#define IMM_SA     as.ue->emitInt32(as.ue->mergeLitstr(read_litstr(as)))
#define IMM_RATA   encodeRAT(*as.ue, read_repo_auth_type(as))
#define IMM_I64A   as.ue->emitInt64(read_opcode_arg<int64_t>(as))
#define IMM_DA     as.ue->emitDouble(read_opcode_arg<double>(as))
#define IMM_LA     as.ue->emitIVA(as.getLocalId(  \
                     read_opcode_arg<std::string>(as)))
#define IMM_IA     as.ue->emitIVA(as.getIterId( \
                     read_opcode_arg<int32_t>(as)))
#define IMM_CAR    as.ue->emitIVA(as.getClsRefSlot( \
                     read_opcode_arg<int32_t>(as)))
#define IMM_CAW    as.ue->emitIVA(as.getClsRefSlot( \
                     read_opcode_arg<int32_t>(as)))
#define IMM_OA(ty) as.ue->emitByte(read_subop<ty>(as));
#define IMM_AA     as.ue->emitInt32(as.ue->mergeArray(read_litarray(as)))
#define IMM_LAR    encodeLocalRange(*as.ue, read_local_range(as))

/*
 * There can currently be no more than one immvector per instruction,
 * and we need access to the size of the immediate vector for
 * NUM_POP_*, so the member vector guy exposes a vecImmStackValues
 * integer.
 */
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
  for (auto const& imm : vecImm) {                      \
    labelJumps.emplace_back(imm, as.ue->bcPos());       \
    as.ue->emitInt32(0); /* to be patched */            \
  }                                                     \
} while (0)

#define IMM_SLA do {                                       \
  auto vecImm = read_sswitch_jmpvector(as);                \
  as.ue->emitInt32(vecImm.size());                         \
  for (auto const& pair : vecImm) {                        \
    as.ue->emitInt32(pair.first);                          \
    labelJumps.emplace_back(pair.second, as.ue->bcPos());  \
    as.ue->emitInt32(0); /* to be patched */               \
  }                                                        \
} while(0)

#define IMM_BA do {                                                 \
  labelJumps.emplace_back(                                          \
    read_opcode_arg<std::string>(as),                               \
    as.ue->bcPos()                                                  \
  );                                                                \
  as.ue->emitInt32(0);                                              \
} while (0)

#define IMM_KA encode_member_key(read_member_key(as), *as.ue)

#define NUM_PUSH_NOV 0
#define NUM_PUSH_ONE(a) 1
#define NUM_PUSH_TWO(a,b) 2
#define NUM_PUSH_THREE(a,b,c) 3
#define NUM_PUSH_INS_1(a) 1
#define NUM_POP_NOV 0
#define NUM_POP_ONE(a) 1
#define NUM_POP_TWO(a,b) 2
#define NUM_POP_THREE(a,b,c) 3
#define NUM_POP_MFINAL immIVA[0]
#define NUM_POP_F_MFINAL immIVA[1]
#define NUM_POP_C_MFINAL (immIVA[0] + 1)
#define NUM_POP_V_MFINAL NUM_POP_C_MFINAL
#define NUM_POP_FMANY immIVA[0] /* number of arguments */
#define NUM_POP_CVUMANY immIVA[0] /* number of arguments */
#define NUM_POP_CMANY immIVA[0] /* number of arguments */
#define NUM_POP_SMANY vecImmStackValues

#define O(name, imm, pop, push, flags)                                 \
  void parse_opcode_##name(AsmState& as) {                             \
    UNUSED uint32_t immIVA[4];                                         \
    UNUSED auto const thisOpcode = Op::name;                           \
    UNUSED const Offset curOpcodeOff = as.ue->bcPos();                 \
    std::vector<std::pair<std::string, Offset> > labelJumps;           \
                                                                       \
    TRACE(                                                             \
      4,                                                               \
      "%d\t[%s] %s\n",                                                 \
      as.in.getLineNumber(),                                           \
      as.displayStackDepth().c_str(),                                  \
      #name                                                            \
    );                                                                 \
                                                                       \
    /* Pretend the stack is reachable and empty, same as hphpc */      \
    if (as.currentStackDepth == nullptr) {                             \
      as.enterReachableRegion(0);                                      \
    }                                                                  \
                                                                       \
    if (isFCallStar(Op##name)) {                                       \
      as.endFpi();                                                     \
    }                                                                  \
                                                                       \
    /* Other FCall* functions perform their own bounds checking. */    \
    if (Op##name == OpFCall || Op##name == OpFCallD ||                 \
        Op##name == OpFCallAwait) {                                    \
      as.fe->containsCalls = true;                                     \
    }                                                                  \
                                                                       \
    as.ue->emitOp(Op##name);                                           \
                                                                       \
    UNUSED size_t immIdx = 0;                                          \
    IMM_##imm;                                                         \
                                                                       \
    int stackDelta = NUM_PUSH_##push - NUM_POP_##pop;                  \
    as.adjustStack(stackDelta);                                        \
                                                                       \
    if (isFPush(Op##name)) {                                           \
      as.beginFpi(curOpcodeOff);                                       \
    }                                                                  \
                                                                       \
    for (auto& kv : labelJumps) {                                      \
      as.addLabelJump(kv.first, kv.second, curOpcodeOff);              \
    }                                                                  \
                                                                       \
    /* Stack depth should be 0 after RetC or RetV. */                  \
    if (thisOpcode == OpRetC || thisOpcode == OpRetV) {                \
      as.enforceStackDepth(0);                                         \
    }                                                                  \
                                                                       \
    /* Stack depth should be 1 after resume from suspend. */           \
    if (thisOpcode == OpCreateCont || thisOpcode == OpAwait ||         \
        thisOpcode == OpYield || thisOpcode == OpYieldK ||             \
        thisOpcode == OpYieldFromDelegate) {                           \
      as.enforceStackDepth(1);                                         \
    }                                                                  \
                                                                       \
    /* Record source location. */                                      \
    as.ue->recordSourceLocation(as.srcLoc, curOpcodeOff);              \
                                                                       \
    if (Op##name == OpDefCls || Op##name == OpDefClsNop) {             \
      as.defClsOffsets.emplace(immIVA[0], curOpcodeOff);               \
    }                                                                  \
                                                                       \
    /* Retain stack depth after calls to exit */                       \
    if ((instrFlags(thisOpcode) & InstrFlags::TF) &&                   \
        (Op##name != OpExit)) {                                        \
      as.enterUnreachableRegion();                                     \
    }                                                                  \
  }

OPCODES

#undef O

#undef IMM_I64A
#undef IMM_SA
#undef IMM_RATA
#undef IMM_DA
#undef IMM_IVA
#undef IMM_LA
#undef IMM_CAR
#undef IMM_CAW
#undef IMM_BA
#undef IMM_BLA
#undef IMM_SLA
#undef IMM_OA
#undef IMM_MA
#undef IMM_AA
#undef IMM_VSA
#undef IMM_KA
#undef IMM_LAR

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
#undef NUM_POP_MFINAL
#undef NUM_POP_F_MFINAL
#undef NUM_POP_C_MFINAL
#undef NUM_POP_V_MFINAL
#undef NUM_POP_FMANY
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
 * maybe-long-string-literal : long-string-literal
 *                           |
 *                           ;
 */
String parse_maybe_long_string(AsmState& as) {
  as.in.skipWhitespace();

  std::vector<char> buffer;
  if (!as.in.readLongString(buffer)) {
    return StrNR(staticEmptyString());
  }
  if (buffer.empty()) {
    return StrNR(staticEmptyString());
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
 * maybe-php-serialized : maybe-long-string-literal
 *                      ;
 */
Variant parse_maybe_php_serialized(AsmState& as) {
  auto s = parse_maybe_long_string(as);
  if (!s.empty()) return unserialize_from_string(s);
  return Variant();
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

/*
 * directive-numclsrefslots : integer ';'
 *                          ;
 */
void parse_numclsrefslots(AsmState& as) {
  if (as.numClsRefSlotsSet) {
    as.error("only one .numclsrefslots directive may appear "
             "in a given function");
  }
  int32_t count = read_opcode_arg<int32_t>(as);
  as.numClsRefSlotsSet = true;
  as.fe->setNumClsRefSlots(count);
  as.in.expectWs(';');
}

/*
 * directive-declvars : var-name* ';'
 *                    ;
 *
 * Variables are usually allocated when first seen, but
 * declvars can be used to preallocate varibles for when
 * the exact assignment matters (like for closures).
 */
void parse_declvars(AsmState& as) {
  std::string var;
  while (as.in.readword(var)) {
    as.getLocalId(var);
  }
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

  auto& eh = as.fe->addEHEnt();
  eh.m_type = EHEnt::Type::Fault;
  eh.m_base = start;
  eh.m_past = as.ue->bcPos();
  eh.m_iterId = iterId;
  eh.m_end = kInvalidOffset;

  as.addLabelEHEnt(label, as.fe->ehtab.size() - 1);
}

/*
 * directive-catch : identifier integer? '{' function-body
 *                 ;
 */
void parse_catch(AsmState& as, int nestLevel) {
  const Offset start = as.ue->bcPos();

  std::string label;
  if (!as.in.readword(label)) {
    as.error("expected label name after .try_catch");
  }
  int iterId = -1;
  as.in.skipWhitespace();
  if (as.in.peek() != '{') {
    iterId = read_opcode_arg<int32_t>(as);
  }
  as.in.expectWs('{');
  parse_function_body(as, nestLevel + 1);

  auto& eh = as.fe->addEHEnt();
  eh.m_type = EHEnt::Type::Catch;
  eh.m_base = start;
  eh.m_past = as.ue->bcPos();
  eh.m_iterId = iterId;
  eh.m_end = kInvalidOffset;

  as.addLabelEHEnt(label, as.fe->ehtab.size() - 1);
}

/*
 * directive-try-catch : integer? '{' function-body ".catch" '{' function-body
 *                     ;
 */
void parse_try_catch(AsmState& as, int nestLevel) {
  const Offset start = as.ue->bcPos();

  int iterId = -1;
  as.in.skipWhitespace();
  if (as.in.peek() != '{') {
    iterId = read_opcode_arg<int32_t>(as);
  }

  // Emit try body.
  as.in.expectWs('{');
  parse_function_body(as, nestLevel + 1);
  if (!as.isUnreachable()) {
    as.error("expected .try region to not fall-thru");
  }

  const Offset handler = as.ue->bcPos();

  // Emit catch body.
  as.enterReachableRegion(0);
  as.ue->emitOp(OpCatch);
  as.adjustStack(1);
  as.enforceStackDepth(1);

  std::string word;
  as.in.skipWhitespace();
  if (!as.in.readword(word) || word != ".catch") {
    as.error("expected .catch directive after .try");
  }
  as.in.skipWhitespace();
  as.in.expectWs('{');
  parse_function_body(as, nestLevel + 1);

  const Offset end = as.ue->bcPos();

  auto& eh = as.fe->addEHEnt();
  eh.m_type = EHEnt::Type::Catch;
  eh.m_base = start;
  eh.m_past = handler;
  eh.m_iterId = iterId;
  eh.m_handler = handler;
  eh.m_end = end;
}

/*
 * directive-srcloc : line_no ':' chr_no ',' line_no ':' chr_no ';'
 *                  ;
 * line_no : integer
 *         ;
 * chr_no  : integer
 *         ;
 *
 * Record that subsequent bytecodes are at the source location indicated by the
 * range of inline numbers and character positions specified.
 */
void parse_srcloc(AsmState& as, int /*nestLevel*/) {
  auto const line0 = as.in.readint();
  as.in.expectWs(':');
  auto const char0 = as.in.readint();
  as.in.expectWs(',');
  auto const line1 = as.in.readint();
  as.in.expectWs(':');
  auto const char1 = as.in.readint();
  as.in.expectWs(';');

  as.srcLoc = Location::Range(line0, char0, line1, char1);
}

/*
 * directive-static : '$' local_name = long-string-literal ';'
 *                  ;
 *
 * Record that the function contains a static named local_name along with an
 * associated initializer.
 */
void parse_static(AsmState& as) {
  Func::SVInfo svInfo;
  std::string name;
  String init;

  as.in.expectWs('$');
  if (!as.in.readword(name)) {
    as.error("Statics must be named");
  }
  svInfo.name = makeStaticString(name);
  as.fe->staticVars.push_back(svInfo);

  as.in.expectWs(';');
}

/*
 * directive-doccomment : long-string-literal ';'
 *                      ;
 *
 */
void parse_func_doccomment(AsmState& as) {
  auto const doc = parse_long_string(as);
  as.in.expectWs(';');

  as.fe->docComment = makeStaticString(doc);
}

/*
 * function-body :  fbody-line* '}'
 *               ;
 *
 * fbody-line :  ".numiters" directive-numiters
 *            |  ".numclsrefslots" directive-numclsrefslots
 *            |  ".declvars" directive-declvars
 *            |  ".try_fault" directive-fault
 *            |  ".try_catch" directive-catch
 *            |  ".try" directive-try-catch
 *            |  ".ismemoizewrapper"
 *            |  ".dyncallwrapper" string-literal
 *            |  ".srcloc" directive-srcloc
 *            |  ".doc" directive-doccomment
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
      if (word == ".ismemoizewrapper") {
        as.fe->isMemoizeWrapper = true;
        as.in.expectWs(';');
        continue;
      }
      if (word == ".dyncallwrapper") {
        as.dynCallWrappers.emplace(as.fe, read_litstr(as));
        as.in.expectWs(';');
        continue;
      }
      if (word == ".numiters")  { parse_numiters(as); continue; }
      if (word == ".declvars")  { parse_declvars(as); continue; }
      if (word == ".numclsrefslots") { parse_numclsrefslots(as); continue; }
      if (word == ".try_fault") { parse_fault(as, nestLevel); continue; }
      if (word == ".try_catch") { parse_catch(as, nestLevel); continue; }
      if (word == ".try") { parse_try_catch(as, nestLevel); continue; }
      if (word == ".srcloc") { parse_srcloc(as, nestLevel); continue; }
      if (word == ".static") { parse_static(as); continue; }
      if (word == ".doc") { parse_func_doccomment(as); continue; }
      as.error("unrecognized directive `" + word + "' in function");
    }
    if (as.in.peek() == ':') {
      as.in.getc();
      as.addLabelTarget(word);
      continue;
    }

    // Ok, it better be an opcode now.
    auto it = opcode_parsers.find(word);
    if (it == opcode_parsers.end()) {
      as.error("unrecognized opcode `" + word + "'");
    }
    it->second(as);

    as.in.skipSpaceTab();
    if (as.in.peek() != '\n' &&
        as.in.peek() != '\r' &&
        as.in.peek() != '#' &&
        as.in.peek() != EOF) {
      as.error("too many arguments for opcode `" + word + "'");
    }
  }
}

void parse_user_attribute(AsmState& as,
                          UserAttributeMap& userAttrs) {
  auto name = read_litstr(as);
  as.in.expectWs('(');

  TypedValue tvInit;
  tvWriteNull(&tvInit); // Don't confuse Variant with uninit data
  tvAsVariant(&tvInit) = parse_php_serialized(as);

  as.in.expectWs(')');

  if (!tvIsArray(&tvInit)) {
    as.error("user attribute values must be arrays");
  }

  tvInit = make_tv<KindOfArray>(ArrayData::GetScalarArray(tvInit.m_data.parr));
  userAttrs[name] = tvInit;
}

/*
 * attribute : attribute-name
 *           | string-literal '(' long-string-literal ')'
 *           ;
 *
 * attribute-list : empty
 *                | '[' attribute* ']'
 *                ;
 *
 * The `attribute-name' rule is context-sensitive; see as-shared.cpp.
 * The second attribute form is for user attributes and only applies
 * if attributeMap is non null.
 */
Attr parse_attribute_list(AsmState& as, AttrContext ctx,
                          UserAttributeMap *userAttrs = nullptr,
                          bool* isTop = nullptr) {
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
    if (as.in.peek() == '"' && userAttrs) {
      parse_user_attribute(as, *userAttrs);
      continue;
    }
    if (!as.in.readword(word)) break;

    auto const abit = string_to_attr(ctx, word);
    if (abit) {
      ret |= *abit;
      continue;
    }
    if (isTop && word == "nontop") {
      *isTop = false;
      continue;
    }

    as.error("unrecognized attribute `" + word + "' in this context");
  }
  as.in.expect(']');
  return Attr(ret);
}

/*
 * type-info       : empty
 *                 | '<' maybe-string-literal maybe-string-literal
 *                       type-flag* '>'
 *                 ;
 * type-constraint : empty
 *                 | '<' maybe-string-literal
 *                       type-flag* '>'
 *                 ;
 * This parses type-info if noUserType is false, type-constraint if true
 */
std::pair<const StringData *, TypeConstraint> parse_type_info(
    AsmState& as, bool noUserType = false) {
  as.in.skipWhitespace();
  if (as.in.peek() != '<') return {};
  as.in.getc();

  const StringData *userType = noUserType ? nullptr : read_maybe_litstr(as);
  const StringData *typeName = read_maybe_litstr(as);

  std::string word;
  auto flags = TypeConstraint::NoFlags;
  for (;;) {
    as.in.skipWhitespace();
    if (as.in.peek() == '>') break;
    if (!as.in.readword(word)) break;

    auto const abit = string_to_type_flag(word);
    if (abit) {
      flags = flags | *abit;
      continue;
    }

    as.error("unrecognized type flag `" + word + "' in this context");
  }
  as.in.expect('>');
  return std::make_pair(userType, TypeConstraint{typeName, flags});
}
TypeConstraint parse_type_constraint(AsmState& as) {
  return parse_type_info(as, true).second;
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

  bool seenVariadic = false;

  for (;;) {
    FuncEmitter::ParamInfo param;

    as.in.skipWhitespace();
    int ch = as.in.peek();
    if (ch == ')') { as.in.getc(); break; } // allow empty param lists

    if (seenVariadic) {
      as.error("functions can only have one variadic argument");
    }

    if (ch == '.') {
      as.in.getc();
      if (as.in.getc() != '.' ||
          as.in.getc() != '.') {
        as.error("expecting '...'");
      }

      seenVariadic = true;
      param.variadic = true;
      as.fe->attrs |= AttrVariadicParam;
    }

    std::tie(param.userType, param.typeConstraint) = parse_type_info(as);

    as.in.skipWhitespace();
    ch = as.in.getc();

    if (ch == '&') {
      param.byRef = true;
      ch = as.in.getc();
    }
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
      if (seenVariadic) {
        as.error("variadic parameter cannot have dv-initializer");
      }

      std::string label;
      if (!as.in.readword(label)) {
        as.error("expected label name for dv-initializer");
      }
      as.addLabelDVInit(label, as.fe->params.size());

      as.in.skipWhitespace();
      ch = as.in.getc();
      if (ch == '(') {
        String str = parse_long_string(as);
        param.phpCode = makeStaticString(str);
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
          param.defaultValue = tv;
        }
        as.in.expectWs(')');
        as.in.skipWhitespace();
        ch = as.in.getc();
      }
    }

    as.fe->appendParam(makeStaticString(name), param);

    if (ch == ')') break;
    if (ch != ',') as.error("expected , between parameter names");
  }
}

void parse_function_flags(AsmState& as) {
  as.in.skipWhitespace();
  std::string flag;
  for (;;) {
    if (as.in.peek() == '{') break;
    if (!as.in.readword(flag)) break;

    if (flag == "isGenerator") {
      as.fe->isGenerator = true;
    } else if (flag == "isAsync") {
      as.fe->isAsync = true;
    } else if (flag == "isClosureBody") {
      as.fe->isClosureBody = true;
    } else if (flag == "isPairGenerator") {
      as.fe->isPairGenerator = true;
    } else {
      as.error("Unexpected function flag \"" + flag + "\"");
    }
  }
}

/*
 * line-range : "(" integer "," integer ")"
 *            ;
 */
std::pair<int,int> parse_line_range(AsmState& as) {
  as.in.skipWhitespace();
  if (as.in.peek() != '(') {
    return std::make_pair(as.in.getLineNumber(), as.in.getLineNumber() + 1);
  }
  as.in.getc();
  int line0 = as.in.readint();
  as.in.expectWs(',');
  int line1 = as.in.readint();
  as.in.expectWs(')');
  return std::make_pair(line0, line1);
}

/*
 * directive-function : attribute-list ?line-range type-info identifier
 *                      parameter-list function-flags '{' function-body
 *                    ;
 */
void parse_function(AsmState& as) {
  if (!as.emittedPseudoMain) {
    as.error(".function blocks must all follow the .main block");
  }

  as.in.skipWhitespace();

  bool isTop = true;

  UserAttributeMap userAttrs;
  Attr attrs = parse_attribute_list(as, AttrContext::Func, &userAttrs, &isTop);

  if(!isTop && as.emittedTopLevelFunc) {
    as.error("All top level functions must be defined after any "
             "non-top functions");
  }

  as.emittedTopLevelFunc |= isTop;

  int line0;
  int line1;
  std::tie(line0, line1) = parse_line_range(as);

  auto typeInfo = parse_type_info(as);
  std::string name;
  if (!as.in.readname(name)) {
    as.error(".function must have a name");
  }

  as.fe = as.ue->newFuncEmitter(makeStaticString(name));
  as.fe->init(line0, line1, as.ue->bcPos(), attrs, isTop, 0);
  std::tie(as.fe->retUserType, as.fe->retTypeConstraint) = typeInfo;
  as.fe->userAttributes = userAttrs;

  parse_parameter_list(as);
  parse_function_flags(as);

  as.in.expectWs('{');

  as.srcLoc = Location::Range{-1,-1,-1,-1};
  parse_function_body(as);
}

/*
 * directive-method : attribute-list ?line-range type-info identifier
 *                      parameter-list function-flags '{' function-body
 *                  ;
 */
void parse_method(AsmState& as) {
  as.in.skipWhitespace();

  UserAttributeMap userAttrs;
  Attr attrs = parse_attribute_list(as, AttrContext::Func, &userAttrs);

  int line0;
  int line1;
  std::tie(line0, line1) = parse_line_range(as);

  auto typeInfo = parse_type_info(as);
  std::string name;
  if (!as.in.readname(name)) {
    as.error(".method requires a method name");
  }

  as.fe = as.ue->newMethodEmitter(makeStaticString(name), as.pce);
  as.pce->addMethod(as.fe);
  as.fe->init(line0, line1,
              as.ue->bcPos(), attrs, false, 0);
  std::tie(as.fe->retUserType, as.fe->retTypeConstraint) = typeInfo;
  as.fe->userAttributes = userAttrs;

  parse_parameter_list(as);
  parse_function_flags(as);

  as.in.expectWs('{');

  as.srcLoc = Location::Range{-1,-1,-1,-1};
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
    if (isStringType(tvInit.m_type)) {
      tvInit.m_data.pstr = makeStaticString(tvInit.m_data.pstr);
      tvInit.m_type = KindOfPersistentString;
      as.ue->mergeLitstr(tvInit.m_data.pstr);
    } else if (isArrayType(tvInit.m_type)) {
      tvInit.m_data.parr = ArrayData::GetScalarArray(tvInit.m_data.parr);
      tvInit.m_type = KindOfPersistentArray;
    } else if (isVecType(tvInit.m_type)) {
      tvInit.m_data.parr = ArrayData::GetScalarArray(tvInit.m_data.parr);
      tvInit.m_type = KindOfPersistentVec;
    } else if (isDictType(tvInit.m_type)) {
      tvInit.m_data.parr = ArrayData::GetScalarArray(tvInit.m_data.parr);
      tvInit.m_type = KindOfPersistentDict;
    } else if (isKeysetType(tvInit.m_type)) {
      tvInit.m_data.parr = ArrayData::GetScalarArray(tvInit.m_data.parr);
      tvInit.m_type = KindOfPersistentKeyset;
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
 * directive-property : attribute-list maybe-long-string-literal type-info
 *                      identifier member-tv-initializer
 *                    ;
 *
 * Define a property with an associated type and heredoc.
 */
void parse_property(AsmState& as) {
  as.in.skipWhitespace();

  Attr attrs = parse_attribute_list(as, AttrContext::Prop);

  auto const heredoc = makeStaticString(parse_maybe_long_string(as));
  auto const userTy = parse_type_info(as, false).first;
  auto const userTyStr = userTy ? userTy : staticEmptyString();

  std::string name;
  if (!as.in.readword(name)) {
    as.error("expected name for property");
  }

  TypedValue tvInit = parse_member_tv_initializer(as);
  as.pce->addProperty(makeStaticString(name),
                      attrs,
                      userTyStr,
                      heredoc,
                      &tvInit,
                      RepoAuthType{});
}

/*
 * const-flags     : isType
 *                 ;
 *
 * directive-const : identifier const-flags member-tv-initializer
 *                 | identifier const-flags ';'
 *                 ;
 */
void parse_constant(AsmState& as) {
  as.in.skipWhitespace();

  std::string name;
  if (!as.in.readword(name)) {
    as.error("expected name for constant");
  }

  bool isType = as.in.tryConsume("isType");
  as.in.skipWhitespace();

  if (as.in.peek() == ';') {
    as.in.getc();
    as.pce->addAbstractConstant(makeStaticString(name),
                                staticEmptyString(),
                                isType);
    return;
  }

  TypedValue tvInit = parse_member_tv_initializer(as);
  as.pce->addConstant(makeStaticString(name),
                      staticEmptyString(), &tvInit,
                      staticEmptyString(),
                      isType);
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
  as.fe->maxStackCells = 1;
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
 * directive-enum_ty : type-constraint ';'
 *                   ;
 *
 */
void parse_enum_ty(AsmState& as) {
  if (as.enumTySet) {
    as.error("only one .enum_ty directive may appear in a given class");
  }
  as.enumTySet = true;

  as.pce->setEnumBaseTy(parse_type_constraint(as));

  as.in.expectWs(';');
}

/*
 * directive-require : 'extends' '<' indentifier '>' ';'
 *                   | 'implements' '<' indentifier '>' ';'
 *                   ;
 *
 */
void parse_require(AsmState& as) {
  as.in.skipWhitespace();

  bool extends = as.in.tryConsume("extends");
  if (!extends && !as.in.tryConsume("implements")) {
    as.error(".require should be extends or implements");
  }

  as.in.expectWs('<');
  std::string name;
  if (!as.in.readname(name)) {
    as.error(".require expects a class or interface name");
  }
  as.in.expectWs('>');

  as.pce->addClassRequirement(PreClass::ClassRequirement(
    makeStaticString(name), extends
  ));

  as.in.expectWs(';');
}

/*
 * directive-doccomment : long-string-literal ';'
 *                      ;
 *
 */
void parse_cls_doccomment(AsmState& as) {
  auto const doc = parse_long_string(as);
  as.in.expectWs(';');

  as.pce->setDocComment(makeStaticString(doc));
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
 *                 | ".enum_ty"      directive-enum-ty
 *                 | ".require"      directive-require
 *                 | ".doc"          directive-doccomment
 *                 ;
 */
void parse_class_body(AsmState& as) {
  if (!as.emittedPseudoMain) {
    as.error(".class blocks must all follow the .main block");
  }

  std::string directive;
  while (as.in.readword(directive)) {
    if (directive == ".method")       { parse_method(as);         continue; }
    if (directive == ".property")     { parse_property(as);       continue; }
    if (directive == ".const")        { parse_constant(as);       continue; }
    if (directive == ".use")          { parse_use(as);            continue; }
    if (directive == ".default_ctor") { parse_default_ctor(as);   continue; }
    if (directive == ".enum_ty")      { parse_enum_ty(as);        continue; }
    if (directive == ".require")      { parse_require(as);        continue; }
    if (directive == ".doc")          { parse_cls_doccomment(as); continue; }

    as.error("unrecognized directive `" + directive + "' in class");
  }
  as.in.expect('}');
}

PreClass::Hoistable compute_hoistable(AsmState& as,
                                      const std::string &name,
                                      const std::string &parentName) {
  auto &pce = *as.pce;
  bool system = pce.attrs() & AttrBuiltin;

  if (pce.methods().size() == 1 && pce.methods()[0]->isClosureBody) {
    return PreClass::NotHoistable;
  }
  if (!system) {
    if (!pce.interfaces().empty() ||
        !pce.usedTraits().empty() ||
        !pce.requirements().empty() ||
        (pce.attrs() & AttrEnum)) {
      return PreClass::Mergeable;
    }
    if (!parentName.empty() && !as.hoistables.count(parentName)) {
      return PreClass::MaybeHoistable;
    }
  }
  as.hoistables.insert(name);

  return pce.attrs() & AttrUnique ?
    PreClass::AlwaysHoistable : PreClass::MaybeHoistable;
}

/*
 * directive-class : ?"top" attribute-list identifier ?line-range
 *                      extension-clause implements-clause '{' class-body
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

  bool isTop = true;

  UserAttributeMap userAttrs;
  Attr attrs = parse_attribute_list(as, AttrContext::Class, &userAttrs, &isTop);
  std::string name;
  if (!as.in.readname(name)) {
    as.error(".class must have a name");
  }

  int line0;
  int line1;
  std::tie(line0, line1) = parse_line_range(as);

  std::string parentName;
  if (as.in.tryConsume("extends")) {
    if (!as.in.readname(parentName)) {
      as.error("expected parent class name after `extends'");
    }
  }

  std::vector<std::string> ifaces;
  if (as.in.tryConsume("implements")) {
    as.in.expectWs('(');
    std::string word;
    while (as.in.readname(word)) {
      ifaces.push_back(word);
    }
    as.in.expect(')');
  }

  auto off = folly::get_default(as.defClsOffsets, as.ue->numPreClasses(),
                                as.ue->bcPos());

  as.pce = as.ue->newBarePreClassEmitter(name, PreClass::MaybeHoistable);
  as.pce->init(line0,
               line1,
               off,
               attrs,
               makeStaticString(parentName),
               staticEmptyString());
  for (auto const& iface : ifaces) {
    as.pce->addInterface(makeStaticString(iface));
  }
  as.pce->setUserAttributes(userAttrs);

  as.in.expectWs('{');
  parse_class_body(as);

  as.pce->setHoistable(
    isTop ? compute_hoistable(as, name, parentName) : PreClass::NotHoistable
  );

  as.finishClass();
}

/*
 * directive-filepath : quoted-string-literal ';'
 *                    ;
 */
void parse_filepath(AsmState& as) {
  auto const str = read_litstr(as);
  as.ue->m_filepath = str;
  as.in.expectWs(';');
}

/*
 * directive-main : ?line-range '{' function-body
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

  int line0;
  int line1;
  std::tie(line0, line1) = parse_line_range(as);

  as.in.expectWs('{');

  as.ue->initMain(line0, line1);
  as.fe = as.ue->getMain();
  as.emittedPseudoMain = true;
  as.srcLoc = Location::Range{-1,-1,-1,-1};
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
 * directive-alias :  attribute-list identifier '=' type-constraint
 *                    maybe-php-serialized ';'
 *                 ;
 *
 * We represent alias type information using the syntax for
 * TypeConstraints. We populate the name and nullable field of the
 * alias directly from the specified type constraint and derive the
 * AnnotType from the compute AnnotType in the constraint.
 *
 * Following the type-constraint we encode the serialized type structure
 * corresponding to this alias.
 */
void parse_alias(AsmState& as) {
  as.in.skipWhitespace();

  Attr attrs = parse_attribute_list(as, AttrContext::Alias);
  std::string name;
  if (!as.in.readname(name)) {
    as.error(".alias must have a name");
  }
  as.in.expectWs('=');

  TypeConstraint ty = parse_type_constraint(as);
  Variant ts = parse_maybe_php_serialized(as);

  if (ts.isInitialized() && !ts.isArray()) {
    as.error(".alias must have an array type structure");
  }

  const StringData* typeName = ty.typeName();
  if (!typeName) typeName = makeStaticString("");
  const StringData* sname = makeStaticString(name);
  // Merge to ensure namedentity creation, according to
  // emitTypedef in emitter.cpp
  as.ue->mergeLitstr(sname);
  as.ue->mergeLitstr(typeName);

  TypeAlias record;
  record.name = sname;
  record.value = typeName;
  record.type = typeName->empty() ? AnnotType::Mixed : ty.type();
  record.nullable = (ty.flags() & TypeConstraint::Nullable) != 0;
  record.attrs = attrs;
  if (ts.isInitialized()) {
    record.typeStructure = ArrNR(ArrayData::GetScalarArray(ts.toArray().get()));
  }
  as.ue->addTypeAlias(record);

  as.in.expectWs(';');
}

void parse_strict(AsmState& as) {
  as.in.skipWhitespace();
  std::string word;
  if (!as.in.readword(word)) {
    as.error(".strict must have a value");
  }
  if (!RuntimeOption::PHP7_ScalarTypes) {
    as.error("Cannot set .strict without PHP7 ScalarTypes");
  }

  as.ue->m_useStrictTypes = as.ue->m_useStrictTypesForBuiltins = word == "1";

  if (!as.ue->m_useStrictTypes && word != "0") {
    as.error("Strict types must be either 1 or 0");
  }

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
 *         |    ".alias"       directive-alias
 *         |    ".strict"      directive-strict
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
    if (directive == ".alias")       { parse_alias(as);    continue; }
    if (directive == ".strict")      { parse_strict(as);   continue; }

    as.error("unrecognized top-level directive `" + directive + "'");
  }

  if (!as.emittedPseudoMain) {
    as.error("no .main found in hhas unit");
  }

  as.resolveDynCallWrappers();
}

}

//////////////////////////////////////////////////////////////////////

std::unique_ptr<UnitEmitter> assemble_string(
  const char* code,
  int codeLen,
  const char* filename,
  const MD5& md5,
  bool swallowErrors
) {
  auto ue = std::make_unique<UnitEmitter>(md5);
  StringData* sd = makeStaticString(filename);
  ue->m_filepath = sd;
  ue->m_useStrictTypes = RuntimeOption::EnableHipHopSyntax ||
                         !RuntimeOption::PHP7_ScalarTypes;

  try {
    auto const mode = std::istringstream::binary | std::istringstream::in;
    std::istringstream instr(std::string(code, codeLen), mode);
    AsmState as(instr);
    as.ue = ue.get();
    parse(as);
  } catch (const std::exception& e) {
    if (!swallowErrors) throw;
    ue = createFatalUnit(sd, md5, FatalOp::Runtime, makeStaticString(e.what()));
  }

  return ue;
}

AsmResult assemble_expression(UnitEmitter& ue, FuncEmitter* fe,
                         int incomingStackDepth,
                         const std::string& expr) {
  auto const mode = std::istringstream::binary | std::istringstream::in;
  std::stringstream sstr(expr + '}', mode);
  AsmState as(sstr);
  as.ue = &ue;
  as.fe = fe;
  as.initStackDepth.adjust(as, incomingStackDepth);
  parse_function_body(as, 1);
  as.finishSection();
  if (as.maxUnnamed >= 0) {
    as.error("Unnamed locals are not allowed in inline assembly");
  }

  if (!as.currentStackDepth) return AsmResult::Unreachable;

  // If we fall off the end of the inline assembly, we're expected to
  // leave a single value on the stack, or leave the stack unchanged.
  if (!as.currentStackDepth->baseValue) {
    as.error("Unknown stack offset on exit from inline assembly");
  }
  auto curStackDepth = as.currentStackDepth->absoluteDepth();
  if (curStackDepth == incomingStackDepth + 1) {
    return AsmResult::ValuePushed;
  }
  if (curStackDepth != incomingStackDepth) {
    as.error("Inline assembly expressions should leave the stack unchanged, "
             "or push exactly one cell onto the stack.");
  }

  return AsmResult::NoResult;
}

//////////////////////////////////////////////////////////////////////

}
