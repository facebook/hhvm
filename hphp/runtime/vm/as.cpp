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
 *   - while class/function names can contains ':', '$', and ';',
 *     .use declarations can't handle those names because of syntax
 *     conflicts
 *
 * @author Jordan DeLong <delong.j@fb.com>
 */

#include "hphp/runtime/vm/as.h"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>

#include <folly/Conv.h>
#include <folly/MapUtil.h>
#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/String.h>

#include "hphp/util/sha1.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/vm/as-shared.h"
#include "hphp/runtime/vm/bc-pattern.h"
#include "hphp/runtime/vm/extern-compiler.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/record-emitter.h"
#include "hphp/runtime/vm/rx.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/system/systemlib.h"
#include "hphp/zend/zend-string.h"

TRACE_SET_MOD(hhas);

namespace HPHP {

AssemblerError::AssemblerError(int where, const std::string& what)
  : std::runtime_error(
      folly::sformat("Assembler Error: line {}: {}", where, what))
{}

//////////////////////////////////////////////////////////////////////

namespace {

StringData* makeDocComment(const String& s) {
  if (RuntimeOption::EvalGenerateDocComments) return makeStaticString(s);
  return staticEmptyString();
}

struct AsmState;
typedef void (*ParserFunc)(AsmState& as);

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
      throw AssemblerError(currentLine,
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
      throw AssemblerError(m_lineNumber, "expected integral value");
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
      assertx(is_hex(i));
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
    while (skipPred(boost::is_any_of(" \t\r\n"))) {
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
  bool skipPred(Predicate pred) {
    while (pred(peek())) {
      if (getc() == EOF) {
        return false;
      }
    }

    return true;
  }

  template<class Predicate, class OutputIterator>
  bool consumePred(Predicate pred, OutputIterator out) {
    int c;
    while (pred(c = peek())) {
      if (getc() == EOF) {
        return false;
      }

      *out++ = c;
    }

    return true;
  }

private:
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
    throw AssemblerError(getLineNumber(), what);
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
    assertx(baseValue.hasValue());
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

struct HashSymbolRef {
  size_t operator()(SymbolRef s) const {
    return static_cast<size_t>(s);
  }
};

struct AsmState {
  explicit AsmState(std::istream& in, bool wants_symbol_refs = false)
    : in{in}
    , wants_symbol_refs{wants_symbol_refs}
  {
    currentStackDepth->setBase(*this, 0);
  }

  AsmState(const AsmState&) = delete;
  AsmState& operator=(const AsmState&) = delete;

  template<typename... Args>
  void error(const std::string& fmt, Args&&... args) {
    throw AssemblerError(in.getLineNumber(),
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

  void finishClass() {
    assertx(!fe && !re);
    ue->addPreClassEmitter(pce);
    pce = 0;
    enumTySet = false;
  }

  void finishRecord() {
    assertx(!fe && !pce);
    ue->addRecordEmitter(re);
    re = nullptr;
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
      fe->numIterators() * kNumIterCells;

    fe->finish(ue->bcPos());

    fe = 0;
    labelMap.clear();
    numItersSet = false;
    initStackDepth = StackDepth();
    initStackDepth.setBase(*this, 0);
    currentStackDepth = &initStackDepth;
    unnamedStackDepths.clear();
    maxUnnamed = -1;
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

  UnitEmitter* ue;
  Input in;
  bool emittedPseudoMain{false};
  bool emittedTopLevelFunc{false};

  /*
   * Map of adata identifiers to their serialized contents
   * Needed because, when instrumenting array provenance, we're unable
   * to initialize their static arrays until the adata is first referenced
   *
   * There's also some painful maneuvering around keeping either the serialized
   * or unserialized array in request heap until it can be made static since
   * this could potentially confusingly OOM a request that autoloads a large
   * unit
   */
  std::unordered_map<std::string, std::vector<char>> adataDecls;

  // Map of adata identifiers to their associated static arrays and potential DV
  // overrides.
  std::map<
    std::string,
    std::pair<ArrayData*,VariableSerializer::DVOverrides>
  > adataMap;
  // Map of array immediates to their adata identifiers.
  std::map<Offset, std::string> adataUses;

  // In whole program mode it isn't possible to lookup a litstr in the global
  // table while emitting, so keep a lookaside of litstrs seen by the assembler.
  std::unordered_map<Id, const StringData*> litstrMap;

  // When inside a class, this state is active.
  PreClassEmitter* pce{nullptr};

  // When inside a record, this state is active.
  RecordEmitter* re{nullptr};

  // When we're doing a function or method body, this state is active.
  FuncEmitter* fe{nullptr};
  std::map<std::string,Label> labelMap;
  bool numItersSet{false};
  bool enumTySet{false};
  StackDepth initStackDepth;
  StackDepth* currentStackDepth{&initStackDepth};
  std::vector<std::unique_ptr<StackDepth>> unnamedStackDepths;
  int minStackDepth{0};
  int maxUnnamed{-1};
  std::set<std::string,stdltistr> hoistables;
  std::unordered_map<uint32_t,Offset> defClsOffsets;
  Location::Range srcLoc{-1,-1,-1,-1};
  hphp_fast_map<SymbolRef,
                CompactVector<std::string>,
                HashSymbolRef> symbol_refs;
  bool wants_symbol_refs;
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
    throw AssemblerError(
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

template<class F>
decltype(auto) suppressOOM(F func) {
  MemoryManager::SuppressOOM so(*tl_heap);
  return func();
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

Variant parse_php_serialized(folly::StringPiece,
                             VariableSerializer::DVOverrides*);

std::pair<ArrayData*, std::string> read_litarray(AsmState& as) {
  as.in.skipSpaceTab();
  if (as.in.getc() != '@') {
    as.error("expecting an `@foo' array literal reference");
  }
  std::string name;
  if (!as.in.readword(name)) {
    as.error("expected name of .adata literal");
  }

  auto adata = [&]() -> ArrayData* {
    auto const it = as.adataMap.find(name);
    if (it != as.adataMap.end()) return it->second.first;
    auto const decl = as.adataDecls.find(name);
    if (decl == as.adataDecls.end()) return nullptr;
    auto& buf = decl->second;
    return suppressOOM([&] {
      VariableSerializer::DVOverrides overrides;
      auto var = parse_php_serialized(
        buf,
        RuntimeOption::EvalHackArrDVArrs ? &overrides : nullptr
      );
      if (!var.isArray()) {
        as.error(".adata only supports serialized arrays");
      }

      auto data = var.detach().m_data.parr;
      ArrayData::GetScalarArray(&data);
      as.adataMap[name] = std::make_pair(data, std::move(overrides));
      as.adataDecls.erase(decl);
      return data;
    });
  }();

  if (!adata) as.error("unknown array data literal name " + name);

  return {adata, std::move(name)};
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
  Y("Cls=",     T::ExactCls);
  Y("?Cls=",    T::OptExactCls);
  Y("?Cls<=",   T::OptSubCls);
  Y("Cls<=",    T::SubCls);
  X("Arr",      T::Arr);
  X("?Arr",     T::OptArr);
  X("VArr",     T::VArr);
  X("?VArr",    T::OptVArr);
  X("DArr",     T::DArr);
  X("?DArr",    T::OptDArr);
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
  X("Func",     T::Func);
  X("?Func",    T::OptFunc);
  X("Cls",      T::Cls);
  X("?Cls",     T::OptCls);
  X("ClsMeth",  T::ClsMeth);
  X("?ClsMeth", T::OptClsMeth);
  X("Record",   T::Record);
  X("?Record",  T::OptRecord);
  X("Ref",      T::Ref);
  X("?Res",     T::OptRes);
  X("Res",      T::Res);
  X("?SArr",    T::OptSArr);
  X("SArr",     T::SArr);
  X("?SVArr",   T::OptSVArr);
  X("SVArr",    T::SVArr);
  X("?SDArr",   T::OptSDArr);
  X("SDArr",    T::SDArr);
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
  X("?UncStrLike",T::OptUncStrLike);
  X("?StrLike",T::OptStrLike);
  X("UncStrLike",T::UncStrLike);
  X("StrLike",T::StrLike);
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
  case T::SVArr:
  case T::OptSVArr:
  case T::VArr:
  case T::OptVArr:
  case T::SDArr:
  case T::OptSDArr:
  case T::DArr:
  case T::OptDArr:
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
  case T::Func:
  case T::OptFunc:
  case T::Cls:
  case T::OptCls:
  case T::ClsMeth:
  case T::OptClsMeth:
  case T::Record:
  case T::OptRecord:
  case T::InitUnc:
  case T::Unc:
  case T::OptUncArrKey:
  case T::OptArrKey:
  case T::UncArrKey:
  case T::ArrKey:
  case T::OptUncStrLike:
  case T::OptStrLike:
  case T::UncStrLike:
  case T::StrLike:
  case T::InitCell:
  case T::Cell:
  case T::Ref:
  case T::InitGen:
  case T::Gen:
  case T::ExactObj:
  case T::SubObj:
  case T::OptExactObj:
  case T::OptSubObj:
  case T::ExactCls:
  case T::SubCls:
  case T::OptExactCls:
  case T::OptSubCls:
    break;
  }

  as.error("unrecognized RepoAuthType format");
  not_reached();
}

// Read a vector of IVAs, with format <int, int, int, ...>, the vector may be
// excluded entirely if it is empty.
std::vector<uint32_t> read_argv32(AsmState& as) {
  as.in.skipSpaceTab();
  if (as.in.peek() != '<') return {};
  as.in.getc();

  std::vector<uint32_t> result;
  for (;;) {
    auto const num = as.in.readint();
    if (num < 0) as.error("Was expecting a positive integer");
    result.push_back(num);
    as.in.skipWhitespace();
    if (as.in.peek() == '>') break;
    as.in.expectWs(',');
  }
  as.in.expectWs('>');

  return result;
}

// Read in a vector of iterators the format for this vector is:
// <(TYPE) ID LOCAL?, (TYPE) ID LOCAL?, ...>
// Where TYPE := Iter | LIter
// and   ID   := Integer
// and   LOCAL := String (only valid when TYPE = LIter)
IterTable read_iter_table(AsmState& as) {
  IterTable ret;

  as.in.skipSpaceTab();
  as.in.expect('<');

  std::string word;

  for (;;) {
    IterTableEnt ent;
    as.in.expectWs('(');
    if (!as.in.readword(word)) as.error("Was expecting Iterator type.");
    if (!word.compare("Iter")) ent.kind = KindOfIter;
    else if (!word.compare("LIter")) ent.kind = KindOfLIter;
    else as.error("Unknown iterator type `" + word + "'");
    as.in.expectWs(')');

    as.in.skipSpaceTab();

    if (!as.in.readword(word)) as.error("Was expecting iterator id.");
    ent.id = as.getIterId(folly::to<uint32_t>(word));

    if (ent.kind == KindOfLIter) {
      as.in.skipSpaceTab();
      if (!as.in.readword(word)) as.error("Was expecting local.");
      ent.local = as.getLocalId(word);
    } else {
      ent.local = kInvalidId;
    }

    ret.push_back(std::move(ent));

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
  auto const count = folly::to<uint32_t>(rest);
  if (!count) return LocalRange{0, 0};
  auto const firstLoc = as.getLocalId(first);
  if (firstLoc + count - 1 > as.maxUnnamed) {
    as.maxUnnamed = firstLoc + count - 1;
  }
  return LocalRange{uint32_t(firstLoc), count};
}

std::pair<FCallArgs::Flags, bool>
read_fcall_flags(AsmState& as, Op thisOpcode) {
  uint8_t flags = 0;
  bool lockWhileUnwinding = false;

  as.in.skipSpaceTab();
  as.in.expect('<');

  std::string flag;
  while (as.in.readword(flag)) {
    if (flag == "SupportsAER") {
      if (thisOpcode == Op::FCallCtor) {
        as.error("FCall flag SupportsAER is not valid for FCallCtor");
      } else {
        flags |= FCallArgs::SupportsAsyncEagerReturn;
        continue;
      }
    }
    if (flag == "LockWhileUnwinding") {
      if (thisOpcode == Op::FCallCtor) {
        lockWhileUnwinding = true;
        continue;
      } else {
        as.error("FCall flag LockWhileUnwinding is only valid for FCallCtor");
      }
    }
    if (flag == "Unpack") { flags |= FCallArgs::HasUnpack; continue; }
    if (flag == "Generics") { flags |= FCallArgs::HasGenerics; continue; }
    as.error("unrecognized FCall flag `" + flag + "'");
  }
  as.in.expectWs('>');

  return std::make_pair(static_cast<FCallArgs::Flags>(flags),
                        lockWhileUnwinding);
}

// Read a vector of booleans formatted as a quoted string of '0' and '1'.
std::unique_ptr<uint8_t[]> read_by_refs(AsmState& as, uint32_t numArgs) {
  as.in.skipSpaceTab();
  std::string strVal;
  if (!as.in.readQuotedStr(strVal)) {
    as.error("expected quoted string literal");
  }

  if (strVal.empty()) return nullptr;
  if (strVal.length() != numArgs) {
    as.error("reffiness vector must be either empty or match number of args");
  }

  auto result = std::make_unique<uint8_t[]>((numArgs + 7) / 8);
  for (auto i = 0; i < numArgs; ++i) {
    auto const c = strVal[i];
    if (c != '0' && c != '1') as.error("Was expecting a boolean (0 or 1)");
    result[i / 8] |= (c == '1' ? 1 : 0) << (i % 8);
  }

  return result;
}

std::tuple<FCallArgsBase, std::unique_ptr<uint8_t[]>, std::string>
read_fcall_args(AsmState& as, Op thisOpcode) {
  FCallArgs::Flags flags;
  bool lockWhileUnwinding;
  std::tie(flags, lockWhileUnwinding) = read_fcall_flags(as, thisOpcode);
  auto const numArgs = read_opcode_arg<uint32_t>(as);
  auto const numRets = read_opcode_arg<uint32_t>(as);
  auto byRefs = read_by_refs(as, numArgs);
  auto asyncEagerLabel = read_opcode_arg<std::string>(as);
  return std::make_tuple(
    FCallArgsBase(flags, numArgs, numRets, lockWhileUnwinding),
    std::move(byRefs),
    std::move(asyncEagerLabel)
  );
}

Id create_litstr_id(AsmState& as) {
  auto const sd = read_litstr(as);
  auto const id = as.ue->mergeLitstr(sd);
  as.litstrMap.emplace(id, sd);
  return id;
}

//////////////////////////////////////////////////////////////////////

std::map<std::string,ParserFunc> opcode_parsers;

#define IMM_NA
#define IMM_ONE(t) IMM_##t
#define IMM_TWO(t1, t2) IMM_ONE(t1); ++immIdx; IMM_##t2
#define IMM_THREE(t1, t2, t3) IMM_TWO(t1, t2); ++immIdx; IMM_##t3
#define IMM_FOUR(t1, t2, t3, t4) IMM_THREE(t1, t2, t3); ++immIdx; IMM_##t4
#define IMM_FIVE(t1, t2, t3, t4, t5) IMM_FOUR(t1, t2, t3, t4); ++immIdx; IMM_##t5
#define IMM_SIX(t1, t2, t3, t4, t5, t6) IMM_FIVE(t1, t2, t3, t4, t5); ++immIdx; IMM_##t6

// Some bytecodes need to know an iva imm for (PUSH|POP)_*.
#define IMM_IVA do {                                      \
    auto imm = read_opcode_arg<uint32_t>(as);             \
    as.ue->emitIVA(imm);                                  \
    immIVA[immIdx] = imm;                                 \
  } while (0)

#define IMM_VSA \
  std::vector<std::string> vecImm = read_strvector(as);                 \
  auto const vecImmStackValues = vecImm.size();                         \
  as.ue->emitIVA(vecImmStackValues);                                    \
  for (size_t i = 0; i < vecImmStackValues; ++i) {                      \
    as.ue->emitInt32(as.ue->mergeLitstr(String(vecImm[i]).get()));      \
  }

#define IMM_SA     as.ue->emitInt32(create_litstr_id(as))
#define IMM_RATA   encodeRAT(*as.ue, read_repo_auth_type(as))
#define IMM_I64A   as.ue->emitInt64(read_opcode_arg<int64_t>(as))
#define IMM_DA     as.ue->emitDouble(read_opcode_arg<double>(as))
#define IMM_LA     as.ue->emitIVA(as.getLocalId(  \
                     read_opcode_arg<std::string>(as)))
#define IMM_IA     as.ue->emitIVA(as.getIterId( \
                     read_opcode_arg<int32_t>(as)))
#define IMM_OA(ty) as.ue->emitByte(read_subop<ty>(as));
#define IMM_LAR    encodeLocalRange(*as.ue, read_local_range(as))
#define IMM_FCA do {                                                \
    auto const fca = read_fcall_args(as, thisOpcode);               \
    encodeFCallArgs(                                                \
      *as.ue, std::get<0>(fca), std::get<1>(fca).get(),             \
      std::get<2>(fca) != "-",                                      \
      [&] {                                                         \
        labelJumps.emplace_back(std::get<2>(fca), as.ue->bcPos());  \
        as.ue->emitInt32(0);                                        \
      }                                                             \
    );                                                              \
    immFCA = std::get<0>(fca);                                      \
  } while (0)

// Record the offset of the immediate so that we can correlate it with its
// associated adata later.
#define IMM_AA do {                             \
  auto const p = read_litarray(as);             \
  auto const pos = as.ue->bcPos();              \
  as.ue->emitInt32(as.ue->mergeArray(p.first)); \
  as.adataUses[pos] = std::move(p.second);      \
} while (0)

/*
 * There can currently be no more than one immvector per instruction,
 * and we need access to the size of the immediate vector for
 * NUM_POP_*, so the member vector guy exposes a vecImmStackValues
 * integer.
 */
#define IMM_ILA do {                               \
  auto const immTable = read_iter_table(as);       \
  as.ue->emitIVA(immTable.size());                 \
  for (auto const& it : immTable) {                \
    as.ue->emitIVA(it.kind);                       \
    as.ue->emitIVA(it.id);                         \
    if (it.kind == KindOfLIter) {                  \
      as.ue->emitIVA(it.local);                    \
    }                                              \
  }                                                \
} while (0)

#define IMM_I32LA do {                             \
  std::vector<uint32_t> vecImm = read_argv32(as);  \
  as.ue->emitIVA(vecImm.size());                   \
  for (auto i : vecImm) {                          \
    as.ue->emitInt32(i);                           \
  }                                                \
} while (0)

#define IMM_BLA do {                                    \
  std::vector<std::string> vecImm = read_jmpvector(as); \
  as.ue->emitIVA(vecImm.size());                        \
  for (auto const& imm : vecImm) {                      \
    labelJumps.emplace_back(imm, as.ue->bcPos());       \
    as.ue->emitInt32(0); /* to be patched */            \
  }                                                     \
} while (0)

#define IMM_SLA do {                                       \
  auto vecImm = read_sswitch_jmpvector(as);                \
  as.ue->emitIVA(vecImm.size());                           \
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
#define NUM_PUSH_CMANY immIVA[0]
#define NUM_PUSH_FCALL immFCA.numRets
#define NUM_PUSH_CALLNATIVE (immIVA[2] + 1)
#define NUM_POP_NOV 0
#define NUM_POP_ONE(a) 1
#define NUM_POP_TWO(a,b) 2
#define NUM_POP_THREE(a,b,c) 3
#define NUM_POP_MFINAL immIVA[0]
#define NUM_POP_C_MFINAL(n) (immIVA[0] + n)
#define NUM_POP_CUMANY immIVA[0] /* number of arguments */
#define NUM_POP_CMANY_U3 immIVA[0] + 3
#define NUM_POP_CALLNATIVE (immIVA[0] + immIVA[2]) /* number of args + nout */
#define NUM_POP_FCALL(nin, nobj) (nin + immFCA.numInputs() + 2 + immFCA.numRets)
#define NUM_POP_CMANY immIVA[0] /* number of arguments */
#define NUM_POP_SMANY vecImmStackValues

#define O(name, imm, pop, push, flags)                                 \
  void parse_opcode_##name(AsmState& as) {                             \
    UNUSED auto immFCA = FCallArgsBase(FCallArgsBase::None, -1, -1,    \
                                       false);                         \
    UNUSED uint32_t immIVA[kMaxHhbcImms];                              \
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
    as.ue->emitOp(Op##name);                                           \
                                                                       \
    UNUSED size_t immIdx = 0;                                          \
    IMM_##imm;                                                         \
                                                                       \
    as.adjustStack(-NUM_POP_##pop);                                    \
                                                                       \
    if (thisOpcode == OpMemoGet) {                                     \
      /* MemoGet pushes after branching */                             \
      assertx(labelJumps.size() == 1);                                 \
      as.addLabelJump(                                                 \
        labelJumps[0].first, labelJumps[0].second, curOpcodeOff        \
      );                                                               \
      as.adjustStack(NUM_PUSH_##push);                                 \
    } else if (thisOpcode == OpMemoGetEager) {                         \
      /* MemoGetEager pushes on its second branch only */              \
      assertx(labelJumps.size() == 2);                                 \
      as.addLabelJump(                                                 \
        labelJumps[0].first, labelJumps[0].second, curOpcodeOff        \
      );                                                               \
      as.adjustStack(NUM_PUSH_##push);                                 \
      as.addLabelJump(                                                 \
        labelJumps[1].first, labelJumps[1].second, curOpcodeOff        \
      );                                                               \
    } else {                                                           \
      /* Everything else pushes before branching */                    \
      as.adjustStack(NUM_PUSH_##push);                                 \
      for (auto& kv : labelJumps) {                                    \
        as.addLabelJump(kv.first, kv.second, curOpcodeOff);            \
      }                                                                \
    }                                                                  \
                                                                       \
    /* FCalls with unpack perform their own bounds checking. */        \
    if (isFCall(Op##name) && !immFCA.hasUnpack()) {                    \
      as.fe->containsCalls = true;                                     \
    }                                                                  \
                                                                       \
    /* Stack depth should be 0 after RetC or RetM. */                  \
    if (thisOpcode == OpRetC || thisOpcode == OpRetCSuspended ||       \
        thisOpcode == OpRetM) {                                        \
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
#undef IMM_BA
#undef IMM_ILA
#undef IMM_I32LA
#undef IMM_BLA
#undef IMM_SLA
#undef IMM_OA
#undef IMM_MA
#undef IMM_AA
#undef IMM_VSA
#undef IMM_KA
#undef IMM_LAR
#undef IMM_FCA

#undef NUM_PUSH_NOV
#undef NUM_PUSH_ONE
#undef NUM_PUSH_TWO
#undef NUM_PUSH_THREE
#undef NUM_PUSH_CMANY
#undef NUM_PUSH_FCALL
#undef NUM_PUSH_CALLNATIVE
#undef NUM_POP_NOV
#undef NUM_POP_ONE
#undef NUM_POP_TWO
#undef NUM_POP_THREE
#undef NUM_POP_MFINAL
#undef NUM_POP_C_MFINAL
#undef NUM_POP_CUMANY
#undef NUM_POP_CMANY_U3
#undef NUM_POP_CALLNATIVE
#undef NUM_POP_FCALL
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

std::vector<char> parse_long_string_raw(AsmState& as) {
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

  return buffer;
}

/*
 * long-string-literal: <string>
 *
 * `long-string-literal' is a python-style longstring.  See
 * readLongString for more details.
 */
String parse_long_string(AsmState& as) {
  auto buffer = parse_long_string_raw(as);
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

void checkSize(TypedValue tv, size_t& available) {
  auto const update = [&] (size_t sz) {
    if (sz > available) {
      throw AssemblerFatal("Maximum allowable size of scalar exceeded");
    }
    available -= sz;
  };

  if (isArrayLikeType(type(tv))) {
    update(allocSize(val(tv).parr));

    IterateKVNoInc(val(tv).parr, [&] (Cell k, TypedValue v) {
      if (isStringType(type(k))) {
        update(val(k).pstr->heapSize());
      }
      checkSize(v, available);
    });
  }

  if (isStringType(type(tv))) {
    update(val(tv).pstr->heapSize());
  }
}

Variant checkSize(Variant val) {
  size_t avail = RuntimeOption::EvalAssemblerMaxScalarSize;
  checkSize(*val.asTypedValue(), avail);
  return val;
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
Variant parse_php_serialized(
  folly::StringPiece str,
  VariableSerializer::DVOverrides* overrides = nullptr
) {
  VariableUnserializer vu(
    str.data(),
    str.size(),
    VariableUnserializer::Type::Internal,
    true
  );
  if (overrides) vu.setDVOverrides(overrides);
  try {
    return checkSize(vu.unserialize());
  } catch (const FatalErrorException&) {
    throw;
  } catch (const AssemblerFatal&) {
    throw;
  } catch (const std::exception& e) {
    auto const msg =
      folly::sformat("AssemblerUnserializationError: {}", e.what());
    throw AssemblerUnserializationError(msg);
  }
}

Variant parse_php_serialized(
  AsmState& as,
  VariableSerializer::DVOverrides* overrides = nullptr
) {
  auto str = parse_long_string(as);
  return parse_php_serialized(str.slice(), overrides);
}

/*
 * maybe-php-serialized : maybe-long-string-literal
 *                      ;
 */
Variant parse_maybe_php_serialized(AsmState& as) {
  auto s = parse_maybe_long_string(as);
  if (!s.empty()) {
    try {
      return unserialize_from_string(s, VariableUnserializer::Type::Internal);
    } catch (const FatalErrorException&) {
      throw;
    } catch (const AssemblerFatal&) {
      throw;
    } catch (const std::exception& e) {
      auto const msg =
        folly::sformat("AssemblerUnserializationError: {}", e.what());
      throw AssemblerUnserializationError(msg);
    }
  }
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
 * directive-declvars : var-name* ';'
 *                    ;
 *
 * Variables are usually allocated when first seen, but
 * declvars can be used to preallocate varibles for when
 * the exact assignment matters (like for closures).
 */
void parse_declvars(AsmState& as) {
  while (true) {
    as.in.skipWhitespace();
    std::string var;
    if (as.in.readQuotedStr(var) || as.in.readword(var)) {
      as.getLocalId(var);
    }
    else {
      break;
    }
  }
  as.in.expectWs(';');
}

void parse_function_body(AsmState&, int nestLevel = 0);

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
 * directive-doccomment : long-string-literal ';'
 *                      ;
 *
 */
void parse_func_doccomment(AsmState& as) {
  auto const doc = parse_long_string(as);
  as.in.expectWs(';');

  as.fe->docComment = makeDocComment(doc);
}

/*
 * fixup_default_values: This function does a *rough* match of the default value
 * initializers for a function and attempts to construct corresponding default
 * TypedValues for them. It will also attempt to normalize the phpCode using a
 * variable serializer.
 */
void fixup_default_values(AsmState& as, FuncEmitter* fe) {
  using Atom = BCPattern::Atom;
  using Captures = BCPattern::CaptureVec;

  auto end = as.ue->bc() + fe->past;
  for (uint32_t paramIdx = 0; paramIdx < fe->params.size(); ++paramIdx) {
    auto& pi = fe->params[paramIdx];
    if (!pi.hasDefaultValue() || pi.funcletOff == kInvalidOffset) continue;
    auto inst = as.ue->bc() + pi.funcletOff;

    // Check that the DV intitializer is actually setting the local for the
    // parameter being initialized.
    auto checkloc = [&] (PC pc, const Captures&) {
      auto const UNUSED op = decode_op(pc);
      assertx(op == OpSetL || op == OpPopL);
      auto const loc = decode_iva(pc);
      return loc == paramIdx;
    };

    // Look for DV initializers which push a primitive value onto the stack and
    // then immediately use it to set the parameter local and pop it from the
    // stack. Currently the following relatively limited sequences are accepted:
    //
    // Int | String | Double | Null | True | False | Array | Dict | Keyset | Vec
    // SetL loc, PopC | PopL loc
    auto result = BCPattern {
      Atom::alt(
        Atom(OpInt), Atom(OpString), Atom(OpDouble), Atom(OpNull), Atom(OpTrue),
        Atom(OpFalse), Atom(OpArray), Atom(OpDict), Atom(OpVec), Atom(OpKeyset)
      ).capture(),
      Atom::alt(
        Atom(OpPopL).onlyif(checkloc),
        Atom::seq(Atom(OpSetL).onlyif(checkloc), Atom(OpPopC))
      ),
    }.ignore({OpAssertRATL, OpAssertRATStk}).matchAnchored(inst, end);

    // Verify that the pattern we matched is either for the last DV initializer,
    // in which case it must end with a JmpNS that targets the function entry,
    // or is immediately followed by the next DV initializer.
    if (!result.found() || result.getEnd() >= end) continue;
    auto pc = result.getEnd();
    auto off = pc - as.ue->bc();
    auto const valid = [&] {
      for (uint32_t next = paramIdx + 1; next < fe->params.size(); ++next) {
        auto& npi = fe->params[next];
        if (!npi.hasDefaultValue() || npi.funcletOff == kInvalidOffset) {
          continue;
        }
        return npi.funcletOff == off;
      }
      auto const orig = pc;
      auto const base = as.ue->bc() + fe->base;
      return decode_op(pc) == OpJmpNS && orig + decode_raw<Offset>(pc) == base;
    }();
    if (!valid) continue;

    // Use the captured initializer bytecode to construct the default value for
    // this parameter.
    auto capture = result.getCapture(0);
    assertx(capture);

    TypedValue dv = make_tv<KindOfUninit>();
    const VariableSerializer::DVOverrides* overrides = nullptr;
    SCOPE_EXIT { overrides = nullptr; };
    auto decode_array = [&] (DataType dt) {
      auto const captureCopy = capture;
      if (auto arr = as.ue->lookupArray(decode_raw<uint32_t>(capture))) {
        dv.m_type = dt;
        dv.m_data.parr = const_cast<ArrayData*>(arr);
        if (RuntimeOption::EvalHackArrDVArrs) {
          auto const litOffset = captureCopy - as.ue->bc();
          auto const it = as.adataUses.find(litOffset);
          assertx(it != as.adataUses.end());
          overrides = &as.adataMap[it->second].second;
        }
      }
    };

    switch (decode_op(capture)) {
    case OpNull:   dv = make_tv<KindOfNull>();           break;
    case OpTrue:   dv = make_tv<KindOfBoolean>(true);    break;
    case OpFalse:  dv = make_tv<KindOfBoolean>(false);   break;
    case OpArray:  decode_array(KindOfPersistentArray);  break;
    case OpVec:    decode_array(KindOfPersistentVec);    break;
    case OpDict:   decode_array(KindOfPersistentDict);   break;
    case OpKeyset: decode_array(KindOfPersistentKeyset); break;
    case OpInt:
      dv = make_tv<KindOfInt64>(decode_raw<int64_t>(capture));
      break;
    case OpDouble:
      dv = make_tv<KindOfDouble>(decode_raw<double>(capture));
      break;
    case OpString:
      if (auto str = as.litstrMap[decode_raw<uint32_t>(capture)]) {
        dv = make_tv<KindOfPersistentString>(str);
      }
      break;
    default:
      always_assert(false);
    }

    // Use the variable serializer to construct a serialized version of the
    // default value, matching the behavior of hphpc.
    if (dv.m_type != KindOfUninit) {
      VariableSerializer vs(VariableSerializer::Type::PHPOutput);
      if (RuntimeOption::EvalHackArrDVArrs && overrides) {
        vs.setDVOverrides(overrides);
      }
      auto str = vs.serialize(tvAsCVarRef(&dv), true);
      pi.defaultValue = dv;
      pi.phpCode = makeStaticString(str.get());
    }
  }
}

/*
 * function-body :  fbody-line* '}'
 *               ;
 *
 * fbody-line :  ".numiters" directive-numiters
 *            |  ".declvars" directive-declvars
 *            |  ".try_fault" directive-fault
 *            |  ".try_catch" directive-catch
 *            |  ".try" directive-try-catch
 *            |  ".ismemoizewrapper"
 *            |  ".ismemoizewrapperlsb"
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
      if (word == ".ismemoizewrapperlsb") {
        as.fe->isMemoizeWrapper = true;
        as.fe->isMemoizeWrapperLSB = true;
        as.in.expectWs(';');
        continue;
      }
      if (word == ".numiters")  { parse_numiters(as); continue; }
      if (word == ".declvars")  { parse_declvars(as); continue; }
      if (word == ".try_catch") { parse_catch(as, nestLevel); continue; }
      if (word == ".try") { parse_try_catch(as, nestLevel); continue; }
      if (word == ".srcloc") { parse_srcloc(as, nestLevel); continue; }
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
  suppressOOM([&] {
    auto name = read_litstr(as);
    as.in.expectWs('(');

    auto var = parse_php_serialized(as);

    as.in.expectWs(')');

    if (!var.isArray()) {
      as.error("user attribute values must be arrays");
    }

    userAttrs[name] =
      RuntimeOption::EvalHackArrDVArrs
        ? make_tv<KindOfVec>(ArrayData::GetScalarArray(std::move(var)))
        : make_tv<KindOfArray>(ArrayData::GetScalarArray(std::move(var)));
  });
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
  if (as.in.peek() != '[') return Attr(ret);
  as.in.getc();

  std::string word;
  auto seen_rxl = false;
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
    auto const rxAttrs = rxAttrsFromAttrString(word);
    if (rxAttrs != 0) {
      if (seen_rxl) as.error("multiple rx attributes");
      seen_rxl = true;
      ret |= rxAttrs;
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
  bool seenRef = false;

  for (;;) {
    FuncEmitter::ParamInfo param;
    param.byRef = false;
    param.inout = false;

    as.in.skipWhitespace();
    int ch = as.in.peek();
    if (ch == ')') { as.in.getc(); break; } // allow empty param lists

    if (seenVariadic) {
      as.error("functions can only have one variadic argument");
    }

    parse_attribute_list(as, AttrContext::Parameter, &param.userAttributes);

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

    if (as.in.tryConsume("inout")) {
      if (seenVariadic) {
        as.error("inout parameters cannot be variadic");
      }
      if (seenRef) {
        as.error("functions cannot contain both inout and ref parameters");
      }
      param.inout = true;
      as.fe->attrs |= AttrTakesInOutParams;
    }

    std::tie(param.userType, param.typeConstraint) = parse_type_info(as);

    as.in.skipWhitespace();
    ch = as.in.getc();

    if (ch == '&') {
      if (param.variadic) {
        as.error("ref parameters cannot be variadic");
      }
      if (param.inout) {
        as.error("parameters cannot be marked both inout and ref");
      }
      if (as.fe->attrs & AttrTakesInOutParams) {
        as.error("functions cannot contain both inout and ref parameters");
      }
      seenRef = true;
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
        tvWriteUninit(tv);
        if (str.size() == 4) {
          if (!strcasecmp("null", str.data())) {
            tvWriteNull(tv);
          } else if (!strcasecmp("true", str.data())) {
            tv = make_tv<KindOfBoolean>(true);
          }
        } else if (str.size() == 5 && !strcasecmp("false", str.data())) {
          tv = make_tv<KindOfBoolean>(false);
        }
        auto utype = param.typeConstraint.underlyingDataType();
        if (tv.m_type == KindOfUninit &&
            (!utype || *utype == KindOfInt64 || *utype == KindOfDouble)) {
          int64_t ival;
          double dval;
          int overflow = 0;
          auto dt = str.get()->isNumericWithVal(ival, dval, false, &overflow);
          if (overflow == 0) {
            if (dt == KindOfInt64) {
              if (utype == KindOfDouble) tv = make_tv<KindOfDouble>(ival);
              else tv = make_tv<KindOfInt64>(ival);
            } else if (dt == KindOfDouble &&
                       (!utype || utype == KindOfDouble)) {
              tv = make_tv<KindOfDouble>(dval);
            }
          }
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
    } else if (flag == "isRxDisabled") {
      // this relies on attributes being parsed before flags
      if (!funcAttrIsAnyRx(as.fe->attrs)) {
        as.error("isRxDisabled on non-rx func");
      }
      as.fe->isRxDisabled = true;
    } else {
      as.error("Unexpected function flag \"" + flag + "\"");
    }
  }
}

/*
 * line-range : "(" integer "," integer ")"
 *            ;
 */
bool parse_line_range(AsmState& as, int& line0, int& line1) {
  as.in.skipWhitespace();
  if (as.in.peek() != '(') {
    line0 = as.in.getLineNumber();
    line1 = as.in.getLineNumber() + 1;
    return false;
  }
  as.in.getc();
  line0 = as.in.readint();
  as.in.expectWs(',');
  line1 = as.in.readint();
  as.in.expectWs(')');
  return true;
}

/*
 * If we haven't seen a pseudomain and we are compiling systemlib,
 * add a pseudomain and return true
 * If we haven't seen a pseudomain and we are not compiling systemlib,
 * return false so that the caller can give an assembler error
 * Otherwise, return true
 */
bool ensure_pseudomain(AsmState& as) {
  if (!as.emittedPseudoMain) {
    if (!SystemLib::s_inited) {
      /*
       * The SystemLib::s_hhas_unit is required to be merge-only,
       * and we create the source by concatenating separate .hhas files
       * Rather than choosing one to have the .main directive, we just
       * generate a trivial pseudoMain automatically.
       */
      as.ue->addTrivialPseudoMain();
      as.emittedPseudoMain = true;
    } else {
      return false;
    }
  }
  return true;
}

static StaticString s_native("__Native");

MaybeDataType type_constraint_to_data_type(LowStringPtr user_type,
  const TypeConstraint& tc) {
    if (auto type = tc.typeName()) {
      // in type_annotation.cpp this code uses m_typeArgs
      // as indicator that type can represent one of collection types
      // when we extract data from the constraint we know if type is one of
      // collection types but we don't have direct way to figure out if
      // type used to have type arguments - do it indirectly by checking
      // if name of user type contains '<'
      auto has_type_args =
        user_type && user_type->slice().str().find('<') != std::string::npos;
      return get_datatype(
        type->toCppString(),
        has_type_args,
        false, // no syntactic functions in type annotations
        false, // no xhp type annotation
        false, // no tuples in type annotation
        tc.isNullable(),
        tc.isSoft());
    }
    return folly::none;
}

static const StaticString
  s_AllowStatic("__AllowStatic");


/*
 * Checks whether the current function is native by looking at the user
 * attribute map and sets the isNative flag accoringly
 * If the give function is op code implementation, then isNative is not set
 */
void check_native(AsmState& as, bool is_construct) {
  if (as.fe->userAttributes.count(s_native.get())) {
    as.fe->hniReturnType = is_construct
      ? KindOfNull
      : type_constraint_to_data_type(as.fe->retUserType,
        as.fe->retTypeConstraint);

    as.fe->isNative =
      !(as.fe->parseNativeAttributes(as.fe->attrs) & Native::AttrOpCodeImpl);

    if (as.fe->isNative) {
      auto info = as.fe->getNativeInfo();
      if (!info) {
        if (SystemLib::s_inited) {
          // non-builtin native functions must have a valid binding
          as.error("No NativeFunctionInfo for function {}",
                   as.fe->nativeFullname());
        } else {
          // Allow builtins to have mising NativeFunctionInfo, to support
          // conditional compilation. Calling such a function will Fatal.
        }
      }
    } else {
      // was AttrOpCodeImpl
    }

    // set extra attributes for builtin native functions
    if (!SystemLib::s_inited) {
      as.fe->attrs |= AttrBuiltin | AttrMayUseVV;
    }

    for (auto& pi : as.fe->params) {
      pi.builtinType =
        type_constraint_to_data_type(pi.userType, pi.typeConstraint);
    }
  }
}

/*
 * directive-function : attribute-list ?line-range type-info identifier
 *                      parameter-list function-flags '{' function-body
 *                    ;
 */
void parse_function(AsmState& as) {
  if (!ensure_pseudomain(as)) {
    as.error(".function blocks must all follow the .main block");
  }

  as.in.skipWhitespace();

  bool isTop = true;

  UserAttributeMap userAttrs;
  Attr attrs = parse_attribute_list(as, AttrContext::Func, &userAttrs, &isTop);

  if (!SystemLib::s_inited) {
    attrs |= AttrUnique | AttrPersistent | AttrBuiltin;
  }

  // Be conservative by default. HHBBC can clear it where appropriate.
  attrs |= AttrMayUseVV;

  if(!isTop && as.emittedTopLevelFunc) {
    as.error("All top level functions must be defined after any "
             "non-top functions");
  }

  as.emittedTopLevelFunc |= isTop;

  int line0;
  int line1;
  parse_line_range(as, line0, line1);

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
  // parse_function_flabs relies on as.fe already having valid attrs
  parse_function_flags(as);

  check_native(as, false);

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

  if (!SystemLib::s_inited) {
    attrs |= AttrBuiltin;

    if (!(attrs & AttrStatic) && !userAttrs.count(s_AllowStatic.get())) {
      attrs |= AttrRequiresThis;
    }
  }

  int line0;
  int line1;
  parse_line_range(as, line0, line1);

  auto typeInfo = parse_type_info(as);
  std::string name;
  if (!as.in.readname(name)) {
    as.error(".method requires a method name");
  }

  auto const sname = makeStaticString(name);
  if (as.pce->hasMethod(sname)) {
    as.error("duplicate method name " + sname->toCppString());
  }

  as.fe = as.ue->newMethodEmitter(sname, as.pce);
  as.pce->addMethod(as.fe);
  as.fe->init(line0, line1,
              as.ue->bcPos(), attrs, false, 0);
  std::tie(as.fe->retUserType, as.fe->retTypeConstraint) = typeInfo;
  as.fe->userAttributes = userAttrs;

  parse_parameter_list(as);
  // parse_function_flabs relies on as.fe already having valid attrs
  parse_function_flags(as);

  check_native(as, name == "__construct");

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
  tvWriteNull(tvInit); // Don't confuse Variant with uninit data

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
      tvWriteUninit(tvInit);
      return tvInit;
    }

    suppressOOM([&] {
      tvAsVariant(&tvInit) = parse_php_serialized(as);
      if (tvInit.m_type == KindOfObject) {
        as.error("property initializer can't be an object");
      } else if (tvInit.m_type == KindOfResource) {
        as.error("property initializer can't be a resource");
      } else {
        tvAsVariant(&tvInit).setEvalScalar();
      }
    });
    as.in.expectWs(';');
  } else if (what == ';') {
    // already null
  } else {
    as.error("expected '=' or ';' after property name");
  }

  return tvInit;
}

template<typename AttrValidator, typename Adder>
void parse_prop_or_field_impl(AsmState& as, AttrValidator validate, Adder add) {
  as.in.skipWhitespace();

  UserAttributeMap userAttributes;
  Attr attrs = parse_attribute_list(as, AttrContext::Prop, &userAttributes);
  validate(attrs);

  auto const heredoc = makeStaticString(parse_maybe_long_string(as));

  const StringData* userTy;
  TypeConstraint typeConstraint;
  std::tie(userTy, typeConstraint) = parse_type_info(as, false);
  auto const userTyStr = userTy ? userTy : staticEmptyString();

  std::string name;
  as.in.skipSpaceTab();
  as.in.consumePred(!boost::is_any_of(" \t\r\n#;="),
                    std::back_inserter(name));
  if (name.empty()) {
    as.error("expected name for property or field");
  }

  TypedValue tvInit = parse_member_tv_initializer(as);
  add(makeStaticString(name),
      attrs,
      userTyStr,
      typeConstraint,
      heredoc,
      &tvInit,
      RepoAuthType{},
      userAttributes);
}

/*
 * directive-property : attribute-list maybe-long-string-literal type-info
 *                      identifier member-tv-initializer
 *                    ;
 *
 * Define a property with an associated type and heredoc.
 */
void parse_property(AsmState& as, bool class_is_const) {
  parse_prop_or_field_impl(
    as,
    [&](Attr attrs) {
      if (attrs & AttrIsConst) {
        if (attrs & (AttrLateInit | AttrLateInitSoft)) {
          as.error("const properties may not also be late init");
        }
      } else if (class_is_const && !(attrs & AttrStatic)) {
        as.error("all instance properties of a const class must be const");
      }
    },
    [&](auto&&... args) {
      as.pce->addProperty(std::forward<decltype(args)>(args)...);
    }
  );
}

void parse_record_field(AsmState& as) {
  parse_prop_or_field_impl(
    as,
    [](Attr attrs) {},
    [&](auto&&... args) {
      as.re->addField(std::forward<decltype(args)>(args)...);
    }
  );
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
 * No-op, for backward compat
 */
void parse_default_ctor(AsmState& as) {
  assertx(!as.fe && as.pce);
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
      bool strict = as.in.tryConsume("strict");
      bool async = as.in.tryConsume("async");

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
        attrs,
        strict,
        async));
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

  as.pce->setDocComment(makeDocComment(doc));
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
void parse_class_body(AsmState& as, bool class_is_const) {
  if (!ensure_pseudomain(as)) {
    as.error(".class blocks must all follow the .main block");
  }

  std::string directive;
  while (as.in.readword(directive)) {
    if (directive == ".property") {
      parse_property(as, class_is_const);
      continue;
    }
    if (directive == ".method")       { parse_method(as);         continue; }
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

/*
 * record-body : record-body-line* '}'
 *             ;
 *
 * record-body-line : ".property"  directive-property
 *                  ;
 */
void parse_record_body(AsmState& as) {
  if (!ensure_pseudomain(as)) {
    as.error(".record blocks must all follow the .main block");
  }

  std::string directive;
  while (as.in.readword(directive)) {
    if (directive == ".property") { parse_record_field(as); continue; }

    as.error(folly::to<std::string>("unrecognized directive `",
                                    directive, "` in record"));
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
  if (!SystemLib::s_inited) {
    attrs |= AttrUnique | AttrPersistent | AttrBuiltin;
  }
  if (attrs & AttrIsConst) {
    if (attrs & (AttrEnum | AttrInterface | AttrTrait)) {
      as.error("interfaces, traits and enums may not be const");
    }
    if (!(attrs & AttrForbidDynamicProps)) {
      as.error("const class missing ForbidDynamicProps attribute");
    }
  }

  std::string name;
  if (!as.in.readname(name)) {
    as.error(".class must have a name");
  }
  if (PreClassEmitter::IsAnonymousClassName(name)) {
    // assign unique numbers to anonymous classes
    // they must not be pre-numbered in the hhas
    auto p = name.find(';');
    if (p != std::string::npos) {
      as.error("anonymous class and closure names may not contain ids in hhas");
    }
    name = HPHP::NewAnonymousClassName(name);
  }

  int line0;
  int line1;
  parse_line_range(as, line0, line1);

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
  parse_class_body(as, attrs & AttrIsConst);

  as.pce->setHoistable(
    isTop ? compute_hoistable(as, name, parentName) : PreClass::NotHoistable
  );

  as.finishClass();
}

/*
 * directive-record : attribute identifier ?line-range
 *                      extension-clause '{' record-body
 *                  ;
 *
 * extension-clause : empty
 *                  | "extends" identifier
 *                  ;
 */
void parse_record(AsmState& as) {
  if (!RuntimeOption::EvalHackRecords && !RuntimeOption::EvalHackRecordArrays) {
    as.error("Records not supported");
  }

  as.in.skipWhitespace();

  Attr attrs = parse_attribute_list(as, AttrContext::Class);
  if (!(attrs & AttrFinal)) {
    // parser only sets the final flag. If the final flag is not set,
    // the record is abstract.
    attrs |= AttrAbstract;
  } else if (attrs & AttrAbstract) {
    as.error("A record cannot be both final and abstract");
  }


  std::string name;
  if (!as.in.readname(name)) {
    as.error(".record must have a name");
  }

  int line0;
  int line1;
  parse_line_range(as, line0, line1);

  std::string parentName;
  if (as.in.tryConsume("extends")) {
    if (!as.in.readname(parentName)) {
      as.error("expected parent record name after `extends'");
    }
  }

  as.re = as.ue->newBareRecordEmitter(name);
  as.re->init(line0,
              line1,
              attrs,
              makeStaticString(parentName),
              staticEmptyString());

  as.in.expectWs('{');
  parse_record_body(as);

  as.finishRecord();
}

/*
 * directive-filepath : quoted-string-literal ';'
 *                    ;
 */
void parse_filepath(AsmState& as) {
  auto const str = read_litstr(as);
  if (nullptr == g_hhas_handler) {
    // We don't want to use file path from cached HHAS
    as.ue->m_filepath = str;
  }
  as.in.expectWs(';');
}

/*
 * directive-main : ?line-range '{' function-body
 *                ;
 */
void parse_main(AsmState& as) {
  if (as.emittedPseudoMain) {
    as.error("Multiple .main directives found");
  }

  int line0;
  int line1;
  bool fromSrcLoc = parse_line_range(as, line0, line1);

  as.in.expectWs('{');

  as.ue->initMain(line0, line1);
  as.fe = as.ue->getMain();
  as.emittedPseudoMain = true;
  if (fromSrcLoc) {
    as.srcLoc = Location::Range{line0,0,line1,0};
  } else {
    as.srcLoc = Location::Range{-1,-1,-1,-1};
  }
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
  as.adataDecls[dataLabel] = parse_long_string_raw(as);
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

  TypeAlias record;
  Attr attrs = parse_attribute_list(as, AttrContext::Alias, &record.userAttrs);
  if (!SystemLib::s_inited) {
    attrs |= AttrPersistent;
  }
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
  if (!typeName) typeName = staticEmptyString();
  const StringData* sname = makeStaticString(name);
  // Merge to ensure namedentity creation, according to
  // emitTypedef in emitter.cpp
  as.ue->mergeLitstr(sname);
  as.ue->mergeLitstr(typeName);

  record.name = sname;
  record.value = typeName;
  record.type = typeName->empty() ? AnnotType::Mixed : ty.type();
  record.nullable = (ty.flags() & TypeConstraint::Nullable) != 0;
  record.attrs = attrs;
  if (ts.isInitialized()) {
    record.typeStructure = ArrNR(ArrayData::GetScalarArray(std::move(ts)));
  }
  auto aliasId = as.ue->addTypeAlias(record);
  as.ue->pushMergeableTypeAlias(aliasId);

  as.in.expectWs(';');
}

/*
 * directive-hh-file : '1' ';'
 *                   | '0' ';'
 *                   ;
 */
void parse_hh_file(AsmState& as) {
  as.in.skipWhitespace();
  std::string word;
  if (!as.in.readword(word)) {
    as.error(".hh_file must have a value");
  }
  as.ue->m_isHHFile = word == "1";

  if (!as.ue->m_isHHFile && word != "0") {
    as.error(".hh_file must be either 1 or 0");
  }

  as.in.expectWs(';');
}

/*
 * directive-symbols : '{' identifier identifier* '}'
 */
void parse_symbol_refs(AsmState& as, SymbolRef symbol_kind) {
  as.in.expectWs('{');

  if (as.wants_symbol_refs) {
    while (true) {
      as.in.skipWhitespace();
      std::string symbol;
      as.in.consumePred(!boost::is_any_of(" \t\r\n#}"),
                        std::back_inserter(symbol));
      if (symbol.empty()) {
        break;
      }
      as.symbol_refs[symbol_kind].push_back(symbol);
    }
  } else {
    while (as.in.peek() != '}') {
      as.in.skipWhitespace();
      if (!as.in.skipPred(!boost::is_any_of("#}"))) break;
    }
  }

  as.in.expect('}');
}

/*
 * directive-filepaths : '{' string string* '}'
 */
void parse_includes(AsmState& as) {
  parse_symbol_refs(as, SymbolRef::Include);
}

void parse_constant_refs(AsmState& as) {
  parse_symbol_refs(as, SymbolRef::Constant);
}

void parse_function_refs(AsmState& as) {
  parse_symbol_refs(as, SymbolRef::Function);
}

void parse_class_refs(AsmState& as) {
  parse_symbol_refs(as, SymbolRef::Class);
}

/*
 * directive-metadata : identifier = identifier ';'
 *                    | identifier = quoted-string-literal ';'
 *                    | identifier = long-string-literal ';'
 *                    ;
 */
void parse_metadata(AsmState& as) {
  std::string key;
  if (as.in.readname(key)) {
    as.in.expectWs('=');
    as.in.skipWhitespace();
    auto const value = [&] () -> const StringData* {
      auto ret = parse_maybe_long_string(as);
      if (!ret.empty()) return makeStaticString(ret);
      std::string tmp;
      if (as.in.readQuotedStr(tmp) || as.in.readword(tmp)) {
        return makeStaticString(tmp);
      }
      return nullptr;
    }();
    if (value) {
      as.in.expect(';');
      as.ue->m_metaData.emplace(
        makeStaticString(key),
        make_tv<KindOfPersistentString>(value)
      );
      return;
    }
  }
  as.error(".metadata expects a key = value pair");
}

/*
 * directive-file-attributes : attribute-list ';'
 *                           ;
 */
void parse_file_attributes(AsmState& as) {
  as.in.skipWhitespace();

  parse_attribute_list(as, AttrContext::Func, &(as.ue->m_fileAttributes));

  as.in.expectWs(';');
}

/*
 * asm-file : asm-tld* <EOF>
 *          ;
 *
 * asm-tld :    ".filepath"         directive-filepath
 *         |    ".main"             directive-main
 *         |    ".function"         directive-function
 *         |    ".adata"            directive-adata
 *         |    ".class"            directive-class
 *         |    ".alias"            directive-alias
 *         |    ".hh_file"          directive-hh-file
 *         |    ".includes"         directive-filepaths
 *         |    ".constant_refs"    directive-symbols
 *         |    ".function_refs"    directive-symbols
 *         |    ".class_refs"       directive-symbols
 *         |    ".metadata"         directive-meta-data
 *         |    ".file_attributes"  directive-file-attributes
 *         ;
 */
void parse(AsmState& as) {
  as.in.skipWhitespace();
  std::string directive;

  while (as.in.readword(directive)) {
    if (directive == ".filepath")      { parse_filepath(as)      ; continue; }
    if (directive == ".main")          { parse_main(as)          ; continue; }
    if (directive == ".function")      { parse_function(as)      ; continue; }
    if (directive == ".adata")         { parse_adata(as)         ; continue; }
    if (directive == ".class")         { parse_class(as)         ; continue; }
    if (directive == ".record")        { parse_record(as)        ; continue; }
    if (directive == ".alias")         { parse_alias(as)         ; continue; }
    if (directive == ".hh_file")       { parse_hh_file(as)       ; continue; }
    if (directive == ".includes")      { parse_includes(as)      ; continue; }
    if (directive == ".constant_refs") { parse_constant_refs(as) ; continue; }
    if (directive == ".function_refs") { parse_function_refs(as) ; continue; }
    if (directive == ".class_refs")    { parse_class_refs(as)    ; continue; }
    if (directive == ".metadata")      { parse_metadata(as)      ; continue; }
    if (directive == ".file_attributes") { parse_file_attributes(as); continue;}

    as.error("unrecognized top-level directive `" + directive + "'");
  }

  if (!ensure_pseudomain(as)) {
    as.error("no .main found in hhas unit");
  }

  if (as.symbol_refs.size()) {
    for (auto& ent : as.symbol_refs) {
      as.ue->m_symbol_refs.push_back(std::move(ent));
    }
  }

  if (RuntimeOption::EvalAssemblerFoldDefaultValues) {
    for (auto& fe : as.ue->fevec()) fixup_default_values(as, fe.get());
    for (size_t n = 0; n < as.ue->numPreClasses(); ++n) {
      for (auto fe : as.ue->pce(n)->methods()) fixup_default_values(as, fe);
    }
  }
}

}

//////////////////////////////////////////////////////////////////////

std::unique_ptr<UnitEmitter> assemble_string(
  const char* code,
  int codeLen,
  const char* filename,
  const SHA1& sha1,
  const Native::FuncTable& nativeFuncs,
  bool swallowErrors,
  bool wantsSymbolRefs
) {
  auto const bcSha1 = SHA1{string_sha1(folly::StringPiece(code, codeLen))};
  auto ue = std::make_unique<UnitEmitter>(sha1, bcSha1, nativeFuncs, false);
  if (!SystemLib::s_inited) {
    ue->m_mergeOnly = true;
  }
  StringData* sd = makeStaticString(filename);
  ue->m_filepath = sd;

  try {
    auto const mode = std::istringstream::binary | std::istringstream::in;
    std::istringstream instr(std::string(code, codeLen), mode);
    AsmState as(instr, wantsSymbolRefs);
    as.ue = ue.get();
    parse(as);
  } catch (const FatalErrorException& e) {
    if (!swallowErrors) throw;
    ue = createFatalUnit(sd, sha1, FatalOp::Runtime,
                         makeStaticString(e.what()));
  } catch (const AssemblerError& e) {
    if (!swallowErrors) throw;
    ue = createFatalUnit(sd, sha1, FatalOp::Runtime, makeStaticString(e.what()));
  } catch (const AssemblerFatal& e) {
    if (!swallowErrors) throw;
    ue = createFatalUnit(sd, sha1, FatalOp::Runtime, makeStaticString(e.what()));
  } catch (const std::exception& e) {
    if (!swallowErrors) {
      // assembler should throw only AssemblerErrors and FatalErrorExceptions
      throw AssemblerError(folly::sformat("AssemblerError: {}", e.what()));
    }
    ue = createFatalUnit(sd, sha1, FatalOp::Runtime,
                         makeStaticString(e.what()));
  }

  return ue;
}

//////////////////////////////////////////////////////////////////////

}
