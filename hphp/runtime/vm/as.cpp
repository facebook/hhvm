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
 *      metadata.  Generally this will crash the VM.
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
#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/vm/as-shared.h"
#include "hphp/runtime/vm/bc-pattern.h"
#include "hphp/runtime/vm/coeffects.h"
#include "hphp/runtime/vm/extern-compiler.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/record-emitter.h"
#include "hphp/runtime/vm/type-alias-emitter.h"
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


namespace {

struct FatalUnitError {
  Location::Range pos;
  FatalOp op;
  const std::string msg;
  const StringData* filePath;
};

} // namespace

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
      error("I/O error reading stream: " + folly::errnoStr(errno));
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
    assertx(baseValue.has_value());
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
  explicit AsmState(std::istream& in) : in{in}
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

  void adjustStackHighwater(int16_t depth) {
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
    label.target = fe->bcPos();

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
    pce = 0;
    enumTySet = false;
  }

  void patchLabelOffsets(const Label& label) {
    for (auto const& source : label.sources) {
      fe->emitInt32(label.target - source.second, source.first);
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
      if (label.second.target >= fe->bcPos()) {
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

    if (fe->numLocals() > std::numeric_limits<uint16_t>::max()) {
      error("too many locals");
    }

    if (fe->numIterators() > std::numeric_limits<uint16_t>::max()) {
      error("too many iterators");
    }

    auto const maxStackCells =
      fe->numLocals() +
      fe->numIterators() * kNumIterCells;
    always_assert(std::numeric_limits<int16_t>::max() >
                  fe->maxStackCells + maxStackCells);
    fe->maxStackCells += maxStackCells;

    fe->finish();

    fe = 0;
    labelMap.clear();
    numItersSet = false;
    numClosuresSet = false;
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

  // Map of adata identifiers to their associated static arrays.
  std::map<std::string, ArrayData*> adataMap;

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
  bool numClosuresSet{false};
  bool enumTySet{false};
  StackDepth initStackDepth;
  StackDepth* currentStackDepth{&initStackDepth};
  std::vector<std::unique_ptr<StackDepth>> unnamedStackDepths;
  int minStackDepth{0};
  int maxUnnamed{-1};
  Location::Range srcLoc{-1,-1,-1,-1};
  hphp_fast_map<SymbolRef,
                CompactVector<std::string>,
                HashSymbolRef> symbol_refs;
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

Variant parse_php_serialized(folly::StringPiece);

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
    if (it != as.adataMap.end()) return it->second;
    auto const decl = as.adataDecls.find(name);
    if (decl == as.adataDecls.end()) return nullptr;
    auto& buf = decl->second;
    return suppressOOM([&] {
      auto var = parse_php_serialized(buf);
      if (!var.isArray()) {
        as.error(".adata only supports serialized arrays");
      }

      auto data = var.detach().m_data.parr;
      ArrayData::GetScalarArray(&data);
      as.adataMap[name] = data;
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
    auto const name = makeStaticString(parse.data());  \
    as.ue->mergeLitstr(name);                          \
    return RepoAuthType{tag, name};                    \
  }

  Y("Obj=",     T::ExactObj);
  Y("?Obj=",    T::OptExactObj);
  Y("UninitObj=",T::UninitExactObj);
  Y("?Obj<=",   T::OptSubObj);
  Y("UninitObj<=",T::UninitSubObj);
  Y("Obj<=",    T::SubObj);
  Y("Cls=",     T::ExactCls);
  Y("?Cls=",    T::OptExactCls);
  Y("?Cls<=",   T::OptSubCls);
  Y("Cls<=",    T::SubCls);
  Y("Record=",  T::ExactRecord);
  Y("?Record=", T::OptExactRecord);
  Y("?Record<=",T::OptSubRecord);
  Y("Record<=", T::SubRecord);
  X("Vec",      T::Vec);
  X("?Vec",     T::OptVec);
  X("Dict",     T::Dict);
  X("?Dict",    T::OptDict);
  X("Keyset",   T::Keyset);
  X("?Keyset",  T::OptKeyset);
  X("Bool",     T::Bool);
  X("?Bool",    T::OptBool);
  X("UninitBool",T::UninitBool);
  X("Cell",     T::Cell);
  X("Dbl",      T::Dbl);
  X("?Dbl",     T::OptDbl);
  X("InitCell", T::InitCell);
  X("InitNull", T::InitNull);
  X("InitUnc",  T::InitUnc);
  X("Int",      T::Int);
  X("?Int",     T::OptInt);
  X("UninitInt",T::UninitInt);
  X("Null",     T::Null);
  X("Obj",      T::Obj);
  X("?Obj",     T::OptObj);
  X("UninitObj",T::UninitObj);
  X("Cls",      T::Cls);
  X("?Cls",     T::OptCls);
  X("ClsMeth",  T::ClsMeth);
  X("?ClsMeth", T::OptClsMeth);
  X("Record",   T::Record);
  X("?Record",  T::OptRecord);
  X("LazyCls",     T::LazyCls);
  X("?LazyCls",    T::OptLazyCls);
  X("?Res",     T::OptRes);
  X("Res",      T::Res);
  X("?SVec",    T::OptSVec);
  X("SVec",     T::SVec);
  X("?SDict",   T::OptSDict);
  X("SDict",    T::SDict);
  X("?SKeyset", T::OptSKeyset);
  X("SKeyset",  T::SKeyset);
  X("?SStr",    T::OptSStr);
  X("UninitSStr",T::UninitSStr);
  X("SStr",     T::SStr);
  X("?Str",     T::OptStr);
  X("UninitStr",T::UninitStr);
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
  X("?UncArrKeyCompat", T::OptUncArrKeyCompat);
  X("?ArrKeyCompat",  T::OptArrKeyCompat);
  X("UncArrKeyCompat",T::UncArrKeyCompat);
  X("ArrKeyCompat",   T::ArrKeyCompat);
  X("VecCompat",T::VecCompat);
  X("?VecCompat",T::OptVecCompat);
  X("Uninit",   T::Uninit);
  X("SArrLike", T::SArrLike);
  X("?SArrLike", T::OptSArrLike);
  X("ArrLike", T::ArrLike);
  X("?ArrLike", T::OptArrLike);
  X("ArrLikeCompat", T::ArrLikeCompat);
  X("?ArrLikeCompat", T::OptArrLikeCompat);
  X("Num", T::Num);
  X("?Num", T::OptNum);
  X("InitPrim", T::InitPrim);
  X("NonNull", T::NonNull);
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
  case T::UninitInt:
  case T::Dbl:
  case T::OptDbl:
  case T::Res:
  case T::OptRes:
  case T::Bool:
  case T::OptBool:
  case T::UninitBool:
  case T::SStr:
  case T::OptSStr:
  case T::UninitSStr:
  case T::Str:
  case T::OptStr:
  case T::UninitStr:
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
  case T::SArrLike:
  case T::ArrLike:
  case T::OptSArrLike:
  case T::OptArrLike:
  case T::Obj:
  case T::OptObj:
  case T::UninitObj:
  case T::Func:
  case T::OptFunc:
  case T::Cls:
  case T::OptCls:
  case T::ClsMeth:
  case T::OptClsMeth:
  case T::Record:
  case T::OptRecord:
  case T::LazyCls:
  case T::OptLazyCls:
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
  case T::OptUncArrKeyCompat:
  case T::OptArrKeyCompat:
  case T::UncArrKeyCompat:
  case T::ArrKeyCompat:
  case T::OptVecCompat:
  case T::VecCompat:
  case T::ArrLikeCompat:
  case T::OptArrLikeCompat:
  case T::Num:
  case T::OptNum:
  case T::InitPrim:
  case T::NonNull:
  case T::InitCell:
  case T::Cell:
  case T::ExactObj:
  case T::SubObj:
  case T::OptExactObj:
  case T::OptSubObj:
  case T::UninitExactObj:
  case T::UninitSubObj:
  case T::ExactCls:
  case T::SubCls:
  case T::OptExactCls:
  case T::OptSubCls:
  case T::ExactRecord:
  case T::SubRecord:
  case T::OptExactRecord:
  case T::OptSubRecord:
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
      int32_t lid = as.getLocalId(name);
      ReadOnlyOp op = static_cast<ReadOnlyOp>(read_subop<ReadOnlyOp>(as));
      return MemberKey{mcode, NamedLocal{lid, lid}, op};
    }
    case MEC: case MPC: {
      auto const i = read_opcode_arg<int32_t>(as);
      ReadOnlyOp op = static_cast<ReadOnlyOp>(read_subop<ReadOnlyOp>(as));
      return MemberKey{mcode, i, op};
    }
    case MEI: {
      auto const i = read_opcode_arg<int64_t>(as);
      ReadOnlyOp op = static_cast<ReadOnlyOp>(read_subop<ReadOnlyOp>(as));
      return MemberKey{mcode, i, op};
    }
    case MET: case MPT: case MQT: {
      auto const litstr = read_litstr(as);
      ReadOnlyOp op = static_cast<ReadOnlyOp>(read_subop<ReadOnlyOp>(as));
      return MemberKey{mcode, litstr, op};
    }
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

IterArgs read_iter_args(AsmState& as) {
  auto const iterInt = read_opcode_arg<int32_t>(as);
  auto const iterId = as.getIterId(iterInt);
  auto const keyId = [&]{
    auto const keyStr = read_opcode_arg<std::string>(as);
    if (keyStr == "NK") return IterArgs::kNoKey;
    if (!folly::StringPiece(keyStr).startsWith("K:")) {
      as.error("Expected: NK | K:$local; got: `" + keyStr + "'");
    }
    return as.getLocalId(keyStr.substr(2));
  }();
  auto const valId = [&]{
    auto const valStr = read_opcode_arg<std::string>(as);
    if (!folly::StringPiece(valStr).startsWith("V:")) {
      as.error("Expected: V:$local; got: `" + valStr + "'");
    }
    return as.getLocalId(valStr.substr(2));
  }();
  return IterArgs(IterArgs::Flags::None, iterId, keyId, valId);
}

FCallArgs::Flags read_fcall_flags(AsmState& as, Op thisOpcode) {
  uint8_t flags = 0;

  as.in.skipSpaceTab();
  as.in.expect('<');

  std::string flag;
  while (as.in.readword(flag)) {
    if (flag == "LockWhileUnwinding") {
      if (thisOpcode == Op::FCallCtor) {
        flags |= FCallArgs::LockWhileUnwinding;
        continue;
      } else {
        as.error("FCall flag LockWhileUnwinding is only valid for FCallCtor");
      }
    }
    if (flag == "Unpack") { flags |= FCallArgs::HasUnpack; continue; }
    if (flag == "Generics") { flags |= FCallArgs::HasGenerics; continue; }
    if (flag == "SkipRepack") { flags |= FCallArgs::SkipRepack; continue; }

    as.error("unrecognized FCall flag `" + flag + "'");
  }
  as.in.expectWs('>');

  return static_cast<FCallArgs::Flags>(flags);
}

// Read a vector of booleans formatted as a quoted string of '0' and '1'.
std::unique_ptr<uint8_t[]> read_inouts(AsmState& as, uint32_t numArgs) {
  as.in.skipSpaceTab();
  std::string strVal;
  if (!as.in.readQuotedStr(strVal)) {
    as.error("expected quoted string literal");
  }

  if (strVal.empty()) return nullptr;
  if (strVal.length() != numArgs) {
    as.error("inout-ness vector must be either empty or match number of args");
  }

  auto result = std::make_unique<uint8_t[]>((numArgs + 7) / 8);
  for (auto i = 0; i < numArgs; ++i) {
    auto const c = strVal[i];
    if (c != '0' && c != '1') as.error("Was expecting a boolean (0 or 1)");
    result[i / 8] |= (c == '1' ? 1 : 0) << (i % 8);
  }

  return result;
}

const StringData* read_fca_context(AsmState& as) {
  as.in.skipSpaceTab();
  std::string strVal;
  as.in.expect('"');
  as.in.readname(strVal);
  as.in.expect('"');
  return !strVal.empty() ? makeStaticString(strVal) : nullptr;
}

std::tuple<FCallArgsBase, std::unique_ptr<uint8_t[]>, std::string,
           const StringData*>
read_fcall_args(AsmState& as, Op thisOpcode) {
  auto const flags = read_fcall_flags(as, thisOpcode);
  auto const numArgs = read_opcode_arg<uint32_t>(as);
  auto const numRets = read_opcode_arg<uint32_t>(as);
  auto inoutArgs = read_inouts(as, numArgs);
  auto asyncEagerLabel = read_opcode_arg<std::string>(as);
  auto const ctx = read_fca_context(as);
  return std::make_tuple(
    FCallArgsBase(flags, numArgs, numRets),
    std::move(inoutArgs),
    std::move(asyncEagerLabel),
    ctx
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
    as.fe->emitIVA(imm);                                  \
    immIVA[immIdx] = imm;                                 \
  } while (0)

#define IMM_VSA \
  std::vector<std::string> vecImm = read_strvector(as);                 \
  auto const vecImmStackValues = vecImm.size();                         \
  as.fe->emitIVA(vecImmStackValues);                                    \
  for (size_t i = 0; i < vecImmStackValues; ++i) {                      \
    as.fe->emitInt32(as.ue->mergeLitstr(String(vecImm[i]).get()));      \
  }

#define IMM_SA     as.fe->emitInt32(create_litstr_id(as))
#define IMM_RATA   encodeRAT(*as.fe, read_repo_auth_type(as))
#define IMM_I64A   as.fe->emitInt64(read_opcode_arg<int64_t>(as))
#define IMM_DA     as.fe->emitDouble(read_opcode_arg<double>(as))
#define IMM_LA     as.fe->emitIVA(as.getLocalId(  \
                     read_opcode_arg<std::string>(as)))
#define IMM_NLA    auto const loc = as.getLocalId(        \
                     read_opcode_arg<std::string>(as));   \
                   as.fe->emitNamedLocal(NamedLocal{loc, loc});
#define IMM_ILA    as.fe->emitIVA(as.getLocalId(  \
                     read_opcode_arg<std::string>(as)))
#define IMM_IA     as.fe->emitIVA(as.getIterId( \
                     read_opcode_arg<int32_t>(as)))
#define IMM_OA(ty) as.fe->emitByte(read_subop<ty>(as));
#define IMM_LAR    encodeLocalRange(*as.fe, read_local_range(as))
#define IMM_ITA    encodeIterArgs(*as.fe, read_iter_args(as))
#define IMM_FCA do {                                                    \
    auto const fca = read_fcall_args(as, thisOpcode);                   \
    auto const& fcab = std::get<0>(fca);                                \
    auto const io = std::get<1>(fca).get();                             \
    encodeFCallArgs(                                                    \
      *as.fe, fcab,                                                     \
      io != nullptr,                                                    \
      [&] {                                                             \
        encodeFCallArgsIO(*as.fe, (fcab.numArgs+7)/8, io);              \
      },                                                                \
      std::get<2>(fca) != "-",                                          \
      [&] {                                                             \
        labelJumps.emplace_back(std::get<2>(fca), as.fe->bcPos());      \
        as.fe->emitInt32(0);                                            \
      },                                                                \
      std::get<3>(fca) != nullptr,                                      \
      [&] {                                                             \
        auto const sd = std::get<3>(fca);                               \
        auto const id = as.ue->mergeLitstr(sd);                         \
        as.litstrMap.emplace(id, sd);                                   \
        as.fe->emitInt32(id);                                           \
      }                                                                 \
    );                                                                  \
    immFCA = fcab;                                                      \
  } while (0)

#define IMM_AA do {                             \
  auto const p = read_litarray(as);             \
  as.fe->emitInt32(as.ue->mergeArray(p.first)); \
} while (0)

#define IMM_BLA do {                                    \
  std::vector<std::string> vecImm = read_jmpvector(as); \
  as.fe->emitIVA(vecImm.size());                        \
  for (auto const& imm : vecImm) {                      \
    labelJumps.emplace_back(imm, as.fe->bcPos());       \
    as.fe->emitInt32(0); /* to be patched */            \
  }                                                     \
} while (0)

#define IMM_SLA do {                                       \
  auto vecImm = read_sswitch_jmpvector(as);                \
  as.fe->emitIVA(vecImm.size());                           \
  for (auto const& pair : vecImm) {                        \
    as.fe->emitInt32(pair.first);                          \
    labelJumps.emplace_back(pair.second, as.fe->bcPos());  \
    as.fe->emitInt32(0); /* to be patched */               \
  }                                                        \
} while(0)

#define IMM_BA do {                                                 \
  labelJumps.emplace_back(                                          \
    read_opcode_arg<std::string>(as),                               \
    as.fe->bcPos()                                                  \
  );                                                                \
  as.fe->emitInt32(0);                                              \
} while (0)

#define IMM_KA encode_member_key(read_member_key(as), *as.fe)

#define NUM_PUSH_NOV 0
#define NUM_PUSH_ONE(a) 1
#define NUM_PUSH_TWO(a,b) 2
#define NUM_PUSH_THREE(a,b,c) 3
#define NUM_PUSH_CMANY immIVA[0]
#define NUM_PUSH_FCALL immFCA.numRets
#define NUM_POP_NOV 0
#define NUM_POP_ONE(a) 1
#define NUM_POP_TWO(a,b) 2
#define NUM_POP_THREE(a,b,c) 3
#define NUM_POP_MFINAL immIVA[0]
#define NUM_POP_C_MFINAL(n) (immIVA[0] + n)
#define NUM_POP_CUMANY immIVA[0] /* number of arguments */
#define NUM_POP_FCALL(nin, nobj) (nin + immFCA.numInputs() + \
                                  (kNumActRecCells - 1) + immFCA.numRets)
#define NUM_POP_CMANY immIVA[0] /* number of arguments */
#define NUM_POP_SMANY vecImmStackValues

#define O(name, imm, pop, push, flags)                                 \
  void parse_opcode_##name(AsmState& as) {                             \
    UNUSED auto immFCA = FCallArgsBase(FCallArgsBase::None, -1, -1);   \
    UNUSED uint32_t immIVA[kMaxHhbcImms];                              \
    UNUSED auto const thisOpcode = Op::name;                           \
    UNUSED const Offset curOpcodeOff = as.fe->bcPos();                 \
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
    as.fe->emitOp(Op##name);                                           \
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
        thisOpcode == OpYield || thisOpcode == OpYieldK) {             \
      as.enforceStackDepth(1);                                         \
    }                                                                  \
                                                                       \
    /* Record source location. */                                      \
    as.fe->recordSourceLocation(as.srcLoc, curOpcodeOff);              \
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
#undef IMM_NLA
#undef IMM_ILA
#undef IMM_BA
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
#undef NUM_POP_NOV
#undef NUM_POP_ONE
#undef NUM_POP_TWO
#undef NUM_POP_THREE
#undef NUM_POP_MFINAL
#undef NUM_POP_C_MFINAL
#undef NUM_POP_CUMANY
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

    IterateKV(val(tv).parr, [&] (TypedValue k, TypedValue v) {
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
Variant parse_php_serialized(folly::StringPiece str) {
  VariableUnserializer vu(
    str.data(),
    str.size(),
    VariableUnserializer::Type::Internal,
    true
  );
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

Variant parse_php_serialized(AsmState& as) {
  auto str = parse_long_string(as);
  return parse_php_serialized(str.slice());
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
 * directive-numclosures : integer ';'
 *                       ;
 */
void parse_numclosures(AsmState& as) {
  if (as.numClosuresSet) {
    as.error("only one .numclosures directive may appear in a given function");
  }
  int32_t count = read_opcode_arg<int32_t>(as);
  as.numClosuresSet = true;
  as.fe->setNumClosures(count);
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

/*
 * directive-coeffects_static : coeffect-name* ';'
 *                            ;
 */
void parse_coeffects_static(AsmState& as) {
  while (true) {
    as.in.skipWhitespace();
    std::string name;
    if (!as.in.readword(name)) break;
    as.fe->staticCoeffects.push_back(makeStaticString(name));
  }
  as.in.expectWs(';');
}

/*
 * directive-coeffects_fun_param : param-index* ';'
 *                               ;
 */
void parse_coeffects_fun_param(AsmState& as) {
  while (true) {
    as.in.skipWhitespace();
    auto const pos = read_opcode_arg<uint32_t>(as);
    as.fe->coeffectRules.emplace_back(
      CoeffectRule(CoeffectRule::FunParam{}, pos));
    if (as.in.peek() == ';') break;
  }
  as.in.expectWs(';');
}

/*
 * directive-coeffects_cc_param : param-index ctx-name* ';'
 *                              ;
 */
void parse_coeffects_cc_param(AsmState& as) {
  while (true) {
    as.in.skipWhitespace();
    auto const pos = read_opcode_arg<uint32_t>(as);
    as.in.skipWhitespace();
    std::string name;
    as.in.readword(name);
    as.fe->coeffectRules.emplace_back(
      CoeffectRule(CoeffectRule::CCParam{}, pos, makeStaticString(name)));
    if (as.in.peek() == ';') break;
  }
  as.in.expectWs(';');
}

/*
 * directive-coeffects_cc_this : type-name* ctx-name ';'
 *                             ;
 */
void parse_coeffects_cc_this(AsmState& as) {
  std::vector<LowStringPtr> names;
  std::string name;
  while (as.in.readword(name)) {
    auto sstr = makeStaticString(name);
    if (as.in.peek() == ';') {
      as.fe->coeffectRules.emplace_back(
        CoeffectRule(CoeffectRule::CCThis{}, names, sstr));
      break;
    }
    names.push_back(sstr);
  }
  as.in.expectWs(';');
}

/*
 * directive-coeffects_cc_reified : [isClass] index type-name* ctx-name ';'
 *                                ;
 */
void parse_coeffects_cc_reified(AsmState& as) {
  std::vector<LowStringPtr> names;
  std::string name;

  as.in.skipWhitespace();
  auto const is_class = as.in.tryConsume("isClass");
  as.in.skipWhitespace();
  auto const pos = read_opcode_arg<uint32_t>(as);
  while (as.in.readword(name)) {
    auto sstr = makeStaticString(name);
    if (as.in.peek() == ';') {
      as.fe->coeffectRules.emplace_back(
        CoeffectRule(CoeffectRule::CCReified{}, is_class, pos, names, sstr));
      break;
    }
    names.push_back(sstr);
  }
  as.in.expectWs(';');
}

/*
 * directive-coeffects_closure_parent_scope ';'
 */
void parse_coeffects_closure_parent_scope(AsmState& as) {
  assertx(as.fe->isClosureBody);
  as.fe->coeffectRules.emplace_back(
    CoeffectRule(CoeffectRule::ClosureParentScope{}));
  as.in.expectWs(';');
}

/*
 * directive-coeffects_generator_this ';'
 */
void parse_coeffects_generator_this(AsmState& as) {
  assertx(!SystemLib::s_inited);
  as.fe->coeffectRules.emplace_back(CoeffectRule(CoeffectRule::GeneratorThis{}));
  as.in.expectWs(';');
}

/*
 * directive-coeffects_caller ';'
 */
void parse_coeffects_caller(AsmState& as) {
  as.fe->coeffectRules.emplace_back(CoeffectRule(CoeffectRule::Caller{}));
  as.in.expectWs(';');
}

void parse_function_body(AsmState&, int nestLevel = 0);

/*
 * directive-catch : identifier integer? '{' function-body
 *                 ;
 */
void parse_catch(AsmState& as, int nestLevel) {
  const Offset start = as.fe->bcPos();

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
  eh.m_past = as.fe->bcPos();
  eh.m_iterId = iterId;
  eh.m_end = kInvalidOffset;

  as.addLabelEHEnt(label, as.fe->ehtab.size() - 1);
}

/*
 * directive-try-catch : integer? '{' function-body ".catch" '{' function-body
 *                     ;
 */
void parse_try_catch(AsmState& as, int nestLevel) {
  const Offset start = as.fe->bcPos();

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

  const Offset handler = as.fe->bcPos();

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

  const Offset end = as.fe->bcPos();

  auto& eh = as.fe->addEHEnt();
  eh.m_base = start;
  eh.m_past = handler;
  eh.m_iterId = iterId;
  eh.m_handler = handler;
  eh.m_end = end;
}

Location::Range parse_srcloc_raw(AsmState& as) {
  auto const line0 = as.in.readint();
  as.in.expectWs(':');
  auto const char0 = as.in.readint();
  as.in.expectWs(',');
  auto const line1 = as.in.readint();
  as.in.expectWs(':');
  auto const char1 = as.in.readint();
  return Location::Range(line0, char0, line1, char1);
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
  auto loc = parse_srcloc_raw(as);
  as.in.expectWs(';');
  as.srcLoc = loc;
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

  auto end = fe->bc() + fe->bcPos();
  for (uint32_t paramIdx = 0; paramIdx < fe->params.size(); ++paramIdx) {
    auto& pi = fe->params[paramIdx];
    if (!pi.hasDefaultValue() || pi.funcletOff == kInvalidOffset) continue;
    auto inst = fe->bc() + pi.funcletOff;

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
    // Int | String | Double | Null | True | False | Vec | Dict | Keyset
    // SetL loc, PopC | PopL loc
    auto result = BCPattern {
      Atom::alt(
        Atom(OpInt), Atom(OpString), Atom(OpDouble), Atom(OpNull),
        Atom(OpTrue), Atom(OpFalse), Atom(OpVec), Atom(OpDict), Atom(OpKeyset)
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
    auto off = pc - fe->bc();
    auto const valid = [&] {
      for (uint32_t next = paramIdx + 1; next < fe->params.size(); ++next) {
        auto& npi = fe->params[next];
        if (!npi.hasDefaultValue() || npi.funcletOff == kInvalidOffset) {
          continue;
        }
        return npi.funcletOff == off;
      }
      auto const orig = pc;
      auto const base = fe->bc();
      return decode_op(pc) == OpJmpNS && orig + decode_raw<Offset>(pc) == base;
    }();
    if (!valid) continue;

    // Use the captured initializer bytecode to construct the default value for
    // this parameter.
    auto capture = result.getCapture(0);
    assertx(capture);

    TypedValue dv = make_tv<KindOfUninit>();
    auto const decode_array = [&] {
      if (auto const arr = as.ue->lookupArray(decode_raw<uint32_t>(capture))) {
        dv.m_type = arr->toPersistentDataType();
        dv.m_data.parr = const_cast<ArrayData*>(arr);
      }
    };

    switch (decode_op(capture)) {
    case OpNull:   dv = make_tv<KindOfNull>();           break;
    case OpTrue:   dv = make_tv<KindOfBoolean>(true);    break;
    case OpFalse:  dv = make_tv<KindOfBoolean>(false);   break;
    case OpVec:    decode_array(); break;
    case OpDict:   decode_array(); break;
    case OpKeyset: decode_array(); break;
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
      if (word == ".numclosures") { parse_numclosures(as); continue; }
      if (word == ".coeffects_static") { parse_coeffects_static(as); continue; }
      if (word == ".coeffects_fun_param") { parse_coeffects_fun_param(as); continue; }
      if (word == ".coeffects_cc_param") { parse_coeffects_cc_param(as); continue; }
      if (word == ".coeffects_cc_this") { parse_coeffects_cc_this(as); continue; }
      if (word == ".coeffects_cc_reified") { parse_coeffects_cc_reified(as); continue; }
      if (word == ".coeffects_closure_parent_scope") {
        parse_coeffects_closure_parent_scope(as);
        continue;
      }
      if (word == ".coeffects_generator_this") {
        parse_coeffects_generator_this(as);
        continue;
      }
      if (word == ".coeffects_caller") {
        parse_coeffects_caller(as);
        continue;
      }
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
      make_array_like_tv(ArrayData::GetScalarArray(std::move(var)));
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
                          UserAttributeMap* userAttrs = nullptr) {
  as.in.skipWhitespace();
  int ret = AttrNone;
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

using TParamNameVec = CompactVector<const StringData*>;

TParamNameVec parse_shadowed_tparams(AsmState& as) {
  TParamNameVec ret;
  as.in.skipWhitespace();
  if (as.in.peek() != '{') return ret;
  as.in.getc();
  std::string name;
  for (;;) {
    as.in.skipWhitespace();
    if (as.in.readword(name)) ret.push_back(makeStaticString(name));
    if (as.in.peek() == '}') break;
    as.in.expectWs(',');
  }
  as.in.expectWs('}');
  return ret;
}

using UpperBoundVec = CompactVector<TypeConstraint>;
using UpperBoundMap = std::unordered_map<const StringData*, UpperBoundVec>;

void parse_ub(AsmState& as, UpperBoundMap& ubs) {
  as.in.skipWhitespace();
  if (as.in.peek() != '(') return;
  as.in.getc();
  std::string name;
  if (!as.in.readword(name)) {
    as.error("Type name expected");
  }
  auto nameStr = makeStaticString(name);
  if (!as.in.readword(name) || name != "as") {
    as.error("Expected keyword as");
  }
  for (;;) {
    const auto& tc = parse_type_info(as).second;
    auto& v = ubs[nameStr];
    v.push_back(tc);
    as.in.skipWhitespace();
    if (as.in.peek() != ',') break;
    as.in.getc();
    as.in.skipWhitespace();
  }
  as.in.expectWs(')');
}

/*
 * upper-bound-list : '{' upper-bound-name-list '}'
 *                ;
 *
 * upper-bound-name-list : empty
 *                       | '(' type-name 'as' type-constraint-list ')'
 *                 ;
 */

UpperBoundMap parse_ubs(AsmState& as) {
  UpperBoundMap ret;
  as.in.skipWhitespace();
  if (as.in.peek() != '{') return ret;
  as.in.getc();
  for (;;) {
    parse_ub(as, ret);
    if (as.in.peek() == '}') break;
    as.in.expectWs(',');
  }
  as.in.expect('}');
  return ret;
}

UpperBoundVec getRelevantUpperBounds(const TypeConstraint& tc,
                                     const UpperBoundMap& ubs,
                                     const UpperBoundMap& class_ubs,
                                     const TParamNameVec& shadowed_tparams) {
  UpperBoundVec ret;
  if (!tc.isTypeVar()) return ret;
  auto const typeName = tc.typeName();
  auto it = ubs.find(typeName);
  if (it != ubs.end()) return it->second;
  if (std::find(shadowed_tparams.begin(), shadowed_tparams.end(), typeName) ==
                shadowed_tparams.end()) {
    it = class_ubs.find(typeName);
    if (it != class_ubs.end()) return it->second;
  }
  return UpperBoundVec{};
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
void parse_parameter_list(AsmState& as,
                          const UpperBoundMap& ubs,
                          const UpperBoundMap& class_ubs,
                          const TParamNameVec& shadowed_tparams,
                          bool hasReifiedGenerics) {
  as.in.skipWhitespace();
  if (as.in.peek() != '(') return;
  as.in.getc();

  bool seenVariadic = false;
  bool seenRef = false;

  for (;;) {
    FuncEmitter::ParamInfo param;

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
      param.setFlag(Func::ParamInfo::Flags::Variadic);
      as.fe->attrs |= AttrVariadicParam;
    }

    if (as.in.tryConsume("inout")) {
      if (seenVariadic) {
        as.error("inout parameters cannot be variadic");
      }
      if (seenRef) {
        as.error("functions cannot contain both inout and ref parameters");
      }
      param.setFlag(Func::ParamInfo::Flags::InOut);
    }

    std::tie(param.userType, param.typeConstraint) = parse_type_info(as);
    auto currUBs = getRelevantUpperBounds(param.typeConstraint, ubs,
                                          class_ubs, shadowed_tparams);
    if (currUBs.size() == 1 && !hasReifiedGenerics) {
      applyFlagsToUB(currUBs[0], param.typeConstraint);
      param.typeConstraint = currUBs[0];
    } else if (!currUBs.empty()) {
      param.upperBounds = std::move(currUBs);
      as.fe->hasParamsWithMultiUBs = true;
    }

    as.in.skipWhitespace();
    ch = as.in.getc();

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

static StaticString s_native("__Native");

MaybeDataType type_constraint_to_data_type(
  LowStringPtr user_type,
  const TypeConstraint& tc
) {
    if (auto type = tc.typeName()) {
      // in type_annotation.cpp this code uses m_typeArgs
      // as indicator that type can represent one of collection types
      // when we extract data from the constraint we know if type is one of
      // collection types but we don't have direct way to figure out if
      // type used to have type arguments - do it indirectly by checking
      // if name of user type contains '>' character as in "Vector<int>"
      auto const has_type_args =
        user_type && user_type->slice().rfind('>') != std::string::npos;
      return get_datatype(
        type->toCppString(),
        has_type_args,
        tc.isNullable(),
        tc.isSoft());
    }
    return folly::none;
}

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
    }
    if (!SystemLib::s_inited) as.fe->attrs |= AttrBuiltin;

    for (auto& pi : as.fe->params) {
      pi.builtinType =
        type_constraint_to_data_type(pi.userType, pi.typeConstraint);
    }
  }
}

/*
 * directive-function : upper-bound-list attribute-list ?line-range type-info
 *                      identifier
 *                      parameter-list function-flags '{' function-body
 *                    ;
 */
void parse_function(AsmState& as) {
  as.in.skipWhitespace();

  auto const ubs = parse_ubs(as);

  UserAttributeMap userAttrs;
  Attr attrs = parse_attribute_list(as, AttrContext::Func, &userAttrs);

  if (!SystemLib::s_inited) {
    attrs |= AttrUnique | AttrPersistent | AttrBuiltin;
  }

  int line0;
  int line1;
  parse_line_range(as, line0, line1);

  auto retTypeInfo = parse_type_info(as);
  std::string name;
  if (!as.in.readname(name)) {
    as.error(".function must have a name");
  }

  as.fe = as.ue->newFuncEmitter(makeStaticString(name));
  as.fe->init(line0, line1, attrs, nullptr);

  auto currUBs = getRelevantUpperBounds(retTypeInfo.second, ubs, {}, {});
  auto const hasReifiedGenerics =
    userAttrs.find(s___Reified.get()) != userAttrs.end();
  if (currUBs.size() == 1 && !hasReifiedGenerics) {
    applyFlagsToUB(currUBs[0], retTypeInfo.second);
    retTypeInfo.second = currUBs[0];
  } else if (!currUBs.empty()) {
    as.fe->retUpperBounds = std::move(currUBs);
    as.fe->hasReturnWithMultiUBs = true;
  }

  std::tie(as.fe->retUserType, as.fe->retTypeConstraint) = retTypeInfo;
  as.fe->userAttributes = userAttrs;

  parse_parameter_list(as, ubs, {}, {}, hasReifiedGenerics);
  // parse_function_flabs relies on as.fe already having valid attrs
  parse_function_flags(as);

  check_native(as, false);

  as.in.expectWs('{');

  // Until we get a .srcloc we attribute the code to the whole function
  as.srcLoc = Location::Range{line0, -1, line1, -1};
  parse_function_body(as);
}

/*
 * directive-method : shadowed-tparam-list upper-bound-list attribute-list
 *                      ?line-range type-info identifier
 *                      parameter-list function-flags '{' function-body
 *                  ;
 */
void parse_method(AsmState& as, const UpperBoundMap& class_ubs) {
  as.in.skipWhitespace();

  auto const shadowed_tparams = parse_shadowed_tparams(as);
  auto const ubs = parse_ubs(as);

  UserAttributeMap userAttrs;
  Attr attrs = parse_attribute_list(as, AttrContext::Func, &userAttrs);

  if (!SystemLib::s_inited) attrs |= AttrBuiltin;

  int line0;
  int line1;
  parse_line_range(as, line0, line1);

  auto retTypeInfo = parse_type_info(as);
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
  as.fe->init(line0, line1, attrs, nullptr);

  auto const hasReifiedGenerics =
    userAttrs.find(s___Reified.get()) != userAttrs.end() ||
    as.pce->userAttributes().find(s___Reified.get()) !=
    as.pce->userAttributes().end();

  auto currUBs = getRelevantUpperBounds(retTypeInfo.second, ubs,
                                        class_ubs, shadowed_tparams);
  if (currUBs.size() == 1 && !hasReifiedGenerics) {
    applyFlagsToUB(currUBs[0], retTypeInfo.second);
    retTypeInfo.second = currUBs[0];
  } else if (!currUBs.empty()) {
    as.fe->retUpperBounds = std::move(currUBs);
    as.fe->hasReturnWithMultiUBs = true;
  }

  std::tie(as.fe->retUserType, as.fe->retTypeConstraint) = retTypeInfo;
  as.fe->userAttributes = userAttrs;

  parse_parameter_list(as, ubs, class_ubs,
                       shadowed_tparams, hasReifiedGenerics);
  // parse_function_flabs relies on as.fe already having valid attrs
  parse_function_flags(as);

  check_native(as, name == "__construct");

  as.in.expectWs('{');

  // Until we get a .srcloc we attribute the code to the whole function
  as.srcLoc = Location::Range{line0, -1, line1, -1};
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
void parse_prop_or_field_impl(AsmState& as,
                              AttrValidator validate,
                              Adder add,
                              const UpperBoundMap& class_ubs) {
  as.in.skipWhitespace();

  UserAttributeMap userAttributes;
  Attr attrs = parse_attribute_list(as, AttrContext::Prop, &userAttributes);
  validate(attrs);

  auto const heredoc = makeDocComment(parse_maybe_long_string(as));

  const StringData* userTy;
  TypeConstraint typeConstraint;
  std::tie(userTy, typeConstraint) = parse_type_info(as, false);
  auto const userTyStr = userTy ? userTy : staticEmptyString();

  auto const hasReifiedGenerics =
    userAttributes.find(s___Reified.get()) != userAttributes.end();
  auto ub = getRelevantUpperBounds(typeConstraint, class_ubs, {}, {});
  auto needsMultiUBs = false;
  if (ub.size() == 1 && !hasReifiedGenerics) {
    applyFlagsToUB(ub[0], typeConstraint);
    typeConstraint = ub[0];
  } else if (!ub.empty()) {
    needsMultiUBs = true;
  }
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
      needsMultiUBs ? std::move(ub) : UpperBoundVec{},
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
void parse_property(AsmState& as, bool class_is_const,
                    const UpperBoundMap& class_ubs) {
  parse_prop_or_field_impl(
    as,
    [&](Attr attrs) {
      if (attrs & AttrIsConst) {
        if (attrs & AttrLateInit) {
          as.error("const properties may not also be late init");
        }
      } else if (class_is_const && !(attrs & AttrStatic)) {
        as.error("all instance properties of a const class must be const");
      }
    },
    [&](auto&&... args) {
      as.pce->addProperty(std::forward<decltype(args)>(args)...);
    },
    class_ubs
  );
}

void parse_record_field(AsmState& as) {
  parse_prop_or_field_impl(
    as,
    [](Attr attrs) {},
    [&](auto&&... args) {
      as.re->addField(std::forward<decltype(args)>(args)...);
    },
    {}
  );
}

/*
 * const-flags     : isType
 *                 : isAbstract
 *                 ;
 *
 * directive-const : identifier const-flags member-tv-initializer
 *                 | identifier const-flags ';'
 *                 ;
 */
void parse_class_constant(AsmState& as) {
  as.in.skipWhitespace();

  std::string name;
  if (!as.in.readword(name)) {
    as.error("expected name for constant");
  }

  bool isType = as.in.tryConsume("isType");
  auto const kind =
    isType ? ConstModifiers::Kind::Type : ConstModifiers::Kind::Value;
  as.in.skipWhitespace();
  DEBUG_ONLY bool isAbstract = as.in.tryConsume("isAbstract");
  as.in.skipWhitespace();

  if (as.in.peek() == ';') {
    as.in.getc();
    assertx(isAbstract);
    as.pce->addAbstractConstant(makeStaticString(name),
                                staticEmptyString(),
                                kind,
                                false);
    return;
  }

  TypedValue tvInit = parse_member_tv_initializer(as);
  as.pce->addConstant(makeStaticString(name),
                      staticEmptyString(), &tvInit,
                      staticEmptyString(),
                      kind,
                      false,
                      Array{},
                      isAbstract);
}

/*
 * const-flags     : isAbstract
 *                 ;
 *
 * directive-ctx : identifier const-flags coeffect-name* ';'
 */
void parse_context_constant(AsmState& as) {
  as.in.skipWhitespace();

  std::string name;
  if (!as.in.readword(name)) {
    as.error("expected name for context constant");
  }

  as.in.skipWhitespace();
  bool isAbstract = as.in.tryConsume("isAbstract");

  auto coeffects = PreClassEmitter::Const::CoeffectsVec{};

  while (true) {
    as.in.skipWhitespace();
    std::string coeffect;
    if (!as.in.readword(coeffect)) break;
    coeffects.push_back(makeStaticString(coeffect));
  }

  as.in.expectWs(';');

  // temporarily drop the abstract ones until runtime is fixed
  if (isAbstract && !RuntimeOption::EvalEnableAbstractContextConstants) return;

  DEBUG_ONLY auto added =
    as.pce->addContextConstant(makeStaticString(name), std::move(coeffects),
                               isAbstract);
  assertx(added);
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
void parse_class_body(AsmState& as, bool class_is_const,
                      const UpperBoundMap& class_ubs) {
  std::string directive;
  while (as.in.readword(directive)) {
    if (directive == ".property") {
      parse_property(as, class_is_const, class_ubs);
      continue;
    }
    if (directive == ".method")       { parse_method(as, class_ubs); continue; }
    if (directive == ".const")        { parse_class_constant(as); continue; }
    if (directive == ".use")          { parse_use(as);            continue; }
    if (directive == ".default_ctor") { parse_default_ctor(as);   continue; }
    if (directive == ".enum_ty")      { parse_enum_ty(as);        continue; }
    if (directive == ".require")      { parse_require(as);        continue; }
    if (directive == ".doc")          { parse_cls_doccomment(as); continue; }
    if (directive == ".ctx")          { parse_context_constant(as); continue; }

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
  std::string directive;
  while (as.in.readword(directive)) {
    if (directive == ".property") { parse_record_field(as); continue; }

    as.error(folly::to<std::string>("unrecognized directive `",
                                    directive, "` in record"));
  }
  as.in.expect('}');
}

/*
 * directive-class : upper-bound-list ?"top" attribute-list identifier
 *                   ?line-range extension-clause implements-clause '{'
 *                   class-body
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

  auto ubs = parse_ubs(as);

  UserAttributeMap userAttrs;
  Attr attrs = parse_attribute_list(as, AttrContext::Class, &userAttrs);
  if (!SystemLib::s_inited) {
    attrs |= AttrUnique | AttrPersistent | AttrBuiltin;
  }
  if (attrs & AttrIsConst) {
    if (attrs & (AttrEnum | AttrEnumClass | AttrInterface | AttrTrait)) {
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

  std::vector<std::string> enum_includes;
  if (as.in.tryConsume("enum_includes")) {
    as.in.expectWs('(');
    std::string word;
    while (as.in.readname(word)) {
      enum_includes.push_back(word);
    }
    as.in.expect(')');
  }

  as.pce = as.ue->newPreClassEmitter(name);
  as.pce->init(line0,
               line1,
               attrs,
               makeStaticString(parentName),
               staticEmptyString());
  for (auto const& iface : ifaces) {
    as.pce->addInterface(makeStaticString(iface));
  }
  for (auto const& enum_include : enum_includes) {
    as.pce->addEnumInclude(makeStaticString(enum_include));
  }
  as.pce->setUserAttributes(userAttrs);

  as.in.expectWs('{');
  parse_class_body(as, attrs & AttrIsConst, ubs);

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
  if (!RuntimeOption::EvalHackRecords) {
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

  as.re = as.ue->newRecordEmitter(name);
  as.re->init(line0,
              line1,
              attrs,
              makeStaticString(parentName),
              staticEmptyString());

  as.in.expectWs('{');
  parse_record_body(as);

  assertx(!as.fe && !as.pce);
  as.re = nullptr;
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

  UserAttributeMap userAttrs;
  Attr attrs = parse_attribute_list(as, AttrContext::Alias, &userAttrs);
  if (!SystemLib::s_inited) {
    attrs |= AttrPersistent;
  }
  std::string name;
  if (!as.in.readname(name)) {
    as.error(".alias must have a name");
  }
  as.in.expectWs('=');

  TypeConstraint ty = parse_type_constraint(as);

  int line0;
  int line1;
  parse_line_range(as, line0, line1);

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

  auto te = as.ue->newTypeAliasEmitter(name);
  te->init(
    line0,
    line1,
    attrs,
    typeName,
    typeName->empty() ? AnnotType::Mixed : ty.type(),
    (ty.flags() & TypeConstraint::Nullable) != 0
  );
  te->setUserAttributes(userAttrs);

  if (ts.isInitialized()) {
    te->setTypeStructure(ArrNR(ArrayData::GetScalarArray(std::move(ts))));
  }

  as.in.expectWs(';');
}

void parse_constant(AsmState& as) {
  as.in.skipWhitespace();

  Constant constant;
  Attr attrs = parse_attribute_list(as, AttrContext::Constant);
  if (!SystemLib::s_inited) {
    attrs |= AttrPersistent;
  }

  std::string name;
  if (!as.in.readword(name)) {
    as.error("expected name for constant");
  }

  as.in.skipWhitespace();

  constant.name = makeStaticString(name);
  constant.val = parse_member_tv_initializer(as);
  constant.attrs = attrs;
  if (type(constant.val) == KindOfUninit) {
    constant.val.m_data.pcnt = reinterpret_cast<MaybeCountable*>(Unit::getCns);
  }
  as.ue->addConstant(constant);
}

/*
 * directive-fatal : pos FatalOp string ';'
 */
void parse_fatal(AsmState& as) {
  as.in.skipWhitespace();
  auto pos = parse_srcloc_raw(as);
  as.in.skipWhitespace();
  FatalOp op = static_cast<FatalOp>(read_subop<FatalOp>(as));
  as.in.skipWhitespace();
  std::string msg;
  if (!as.in.readQuotedStr(msg)) {
    as.error(".fatal must have a message");
  }
  as.in.expectWs(';');
  throw FatalUnitError{pos, op, msg, as.ue->m_filepath};
}

/*
 * directive-symbols : '{' identifier identifier* '}'
 */
void parse_symbol_refs(AsmState& as, SymbolRef symbol_kind) {
  as.in.expectWs('{');

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
    if (directive == ".function")      { parse_function(as)      ; continue; }
    if (directive == ".adata")         { parse_adata(as)         ; continue; }
    if (directive == ".class")         { parse_class(as)         ; continue; }
    if (directive == ".record")        { parse_record(as)        ; continue; }
    if (directive == ".alias")         { parse_alias(as)         ; continue; }
    if (directive == ".includes")      { parse_includes(as)      ; continue; }
    if (directive == ".const")         { parse_constant(as)      ; continue; }
    if (directive == ".constant_refs") { parse_constant_refs(as) ; continue; }
    if (directive == ".function_refs") { parse_function_refs(as) ; continue; }
    if (directive == ".class_refs")    { parse_class_refs(as)    ; continue; }
    if (directive == ".metadata")      { parse_metadata(as)      ; continue; }
    if (directive == ".file_attributes") { parse_file_attributes(as); continue;}
    if (directive == ".fatal")         { parse_fatal(as)         ; continue; }

    as.error("unrecognized top-level directive `" + directive + "'");
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
  bool swallowErrors
) {
  tracing::Block _{
    "assemble",
    [&] {
      return tracing::Props{}
        .add("filename", filename)
        .add("code_size", codeLen);
    }
  };

  auto const bcSha1 = SHA1{string_sha1(folly::StringPiece(code, codeLen))};
  auto ue = std::make_unique<UnitEmitter>(sha1, bcSha1, nativeFuncs, false);
  StringData* sd = makeStaticString(filename);
  ue->m_filepath = sd;

  FTRACE(
    4,
    "==================== Assembling {} ====================\n{}\n",
    ue->m_filepath,
    std::string(code, codeLen)
  );

  try {
    auto const mode = std::istringstream::binary | std::istringstream::in;
    std::istringstream instr(std::string(code, codeLen), mode);
    AsmState as{instr};
    as.ue = ue.get();
    parse(as);
  } catch (const FatalUnitError& e) {
    auto const filePath = e.filePath ? e.filePath : sd;
    ue = createFatalUnit(filePath, sha1, e.op, e.msg, e.pos);
  } catch (const FatalErrorException& e) {
    if (!swallowErrors) throw;
    ue = createFatalUnit(sd, sha1, FatalOp::Runtime, e.what());
  } catch (AssemblerError& e) {
    if (!swallowErrors) throw;
    ue = createFatalUnit(sd, sha1, FatalOp::Runtime, e.what());
  } catch (const AssemblerFatal& e) {
    if (!swallowErrors) throw;
    ue = createFatalUnit(sd, sha1, FatalOp::Runtime, e.what());
  } catch (const std::exception& e) {
    if (!swallowErrors) {
      // assembler should throw only AssemblerErrors and FatalErrorExceptions
      throw AssemblerError(folly::sformat("AssemblerError: {}", e.what()));
    }
    ue = createFatalUnit(sd, sha1, FatalOp::Runtime, e.what());
  }

  return ue;
}

//////////////////////////////////////////////////////////////////////

}
