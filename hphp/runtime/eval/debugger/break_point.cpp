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

#include <runtime/eval/debugger/break_point.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/eval/debugger/debugger_proxy.h>
#include <runtime/eval/debugger/debugger_thrift_buffer.h>
#include <runtime/base/preg.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/class_info.h>
#include <runtime/base/stat_cache.h>
#include <runtime/vm/translator/translator-inline.h>

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

static const Trace::Module TRACEMOD = Trace::debugger;

int InterruptSite::getFileLen() const {
  TRACE(2, "InterruptSite::getFileLen\n");
  if (m_file.empty()) {
    getFile();
  }
  return m_file.size();
}

std::string InterruptSite::desc() const {
  TRACE(2, "InterruptSite::desc\n");
  string ret;
  if (m_exception.isNull()) {
    ret = "Break";
  } else if (m_exception.isObject()) {
    ret = "Exception thrown";
  } else {
    ret = "Error occurred";
  }

  const char *cls = getClass();
  const char *func = getFunction();
  if (func && *func) {
    ret += " at ";
    if (cls && *cls) {
      ret += cls;
      ret += "::";
    }
    ret += func;
    ret += "()";
  }

  string file = getFile();
  int line0 = getLine0();
  if (line0) {
    ret += " on line " + lexical_cast<string>(line0);
    if (!file.empty()) {
      ret += " of " + file;
    }
  } else if (!file.empty()) {
    ret += " in " + file;
  }

  return ret;
}

InterruptSite::InterruptSite(bool hardBreakPoint, CVarRef e)
    : m_exception(e), m_class(nullptr), m_function(nullptr),
      m_file((StringData*)nullptr),
      m_line0(0), m_char0(0), m_line1(0), m_char1(0),
      m_unit(nullptr), m_valid(false), m_funcEntry(false) {
  TRACE(2, "InterruptSite::InterruptSite\n");
  VM::Transl::VMRegAnchor _;
#define bail_on(c) if (c) { return; }
  VMExecutionContext* context = g_vmContext;
  ActRec *fp = context->getFP();
  bail_on(!fp);
  Offset offset;
  if (hardBreakPoint && fp->skipFrame()) {
    // for hard breakpoint, the fp is for an extension function,
    // so we need to construct the site on the caller
    fp = context->getPrevVMState(fp, &offset);
    assert(fp);
    bail_on(!fp->m_func);
    m_unit = fp->m_func->unit();
    bail_on(!m_unit);
  } else {
    const uchar *pc = context->getPC();
    bail_on(!fp->m_func);
    m_unit = fp->m_func->unit();
    bail_on(!m_unit);
    offset = m_unit->offsetOf(pc);
    if (offset == fp->m_func->base()) {
      m_funcEntry = true;
    }
  }
  m_file = m_unit->filepath()->data();
  if (m_unit->getSourceLoc(offset, m_sourceLoc)) {
    m_line0 = m_sourceLoc.line0;
    m_char0 = m_sourceLoc.char0;
    m_line1 = m_sourceLoc.line1;
    m_char1 = m_sourceLoc.char1;

    if (g_context->getDebuggerSmallStep()) {
      // get offset range for the pc only
      VM::OffsetRange range;
      if (m_unit->getOffsetRange(offset, range)) {
        m_offsetRangeVec.push_back(range);
      }
    } else {
      // get offset ranges for the whole line
      // we use m_line1 here because it seems working better than m_line0
      // in a handful of cases for our bytecode-source mapping. we probably
      // should consider modify the mapping to make stepping easier.
      if (!m_unit->getOffsetRanges(m_line1, m_offsetRangeVec)) {
        m_offsetRangeVec.clear();
      }
    }
  }
  m_function = fp->m_func->name()->data();
  if (fp->m_func->preClass()) {
    m_class = fp->m_func->preClass()->name()->data();
  } else {
    m_class = "";
  }
#undef bail_on
  m_valid = true;
}

///////////////////////////////////////////////////////////////////////////////

const char *BreakPointInfo::ErrorClassName = "@";

const char *BreakPointInfo::GetInterruptName(InterruptType interrupt) {
  TRACE(2, "BreakPointInfo::GetInterruptName\n");
  switch (interrupt) {
    case RequestStarted: return "start of request";
    case RequestEnded:   return "end of request or start of psp";
    case PSPEnded:       return "end of psp";
    default:
      assert(false);
      break;
  }
  return nullptr;
}

BreakPointInfo::BreakPointInfo(bool regex, State state,
                               const std::string &file, int line)
    : m_index(0), m_state(state), m_valid(true),
      m_interrupt(BreakPointReached),
      m_file(file), m_line1(line), m_line2(line),
      m_regex(regex), m_check(false) {
  TRACE(2, "BreakPointInfo::BreakPointInfo..const std::string &file, int)\n");
  createIndex();
}

BreakPointInfo::BreakPointInfo(bool regex, State state,
                               InterruptType interrupt,
                               const std::string &url)
    : m_index(0), m_state(state), m_valid(true), m_interrupt(interrupt),
      m_line1(0), m_line2(0), m_url(url),
      m_regex(regex), m_check(false) {
  TRACE(2, "BreakPointInfo::BreakPointInfo..const std::string &url)\n");
  createIndex();
}

BreakPointInfo::BreakPointInfo(bool regex, State state,
                               InterruptType interrupt,
                               const std::string &exp,
                               const std::string &file)
    : m_index(0), m_state(state), m_valid(true), m_interrupt(interrupt),
      m_line1(0), m_line2(0),
      m_regex(regex), m_check(false) {
  TRACE(2, "BreakPointInfo::BreakPointInfo..const std::string &file)\n");
  if (m_interrupt == ExceptionThrown) {
    parseExceptionThrown(exp);
    if (!m_file.empty() || m_line1 || m_line2 || !m_funcs.empty()) {
      m_valid = false;
    }
  } else {
    parseBreakPointReached(exp, file);
  }
  createIndex();
}

static int s_max_breakpoint_index = 0;
void BreakPointInfo::createIndex() {
  TRACE(2, "BreakPointInfo::createIndex\n");
  m_index = ++s_max_breakpoint_index;
}

BreakPointInfo::~BreakPointInfo() {
  TRACE(2, "BreakPointInfo::~BreakPointInfo\n");
  if (m_index && m_index == s_max_breakpoint_index) {
    --s_max_breakpoint_index;
  }
}

void BreakPointInfo::sendImpl(DebuggerThriftBuffer &thrift) {
  TRACE(2, "BreakPointInfo::sendImpl\n");
  thrift.write(m_state);
  thrift.write(m_interrupt);
  thrift.write(m_file);
  thrift.write(m_line1);
  thrift.write(m_line2);
  thrift.write(m_namespace);
  thrift.write(m_class);
  thrift.write(m_funcs);
  thrift.write(m_url);
  thrift.write(m_regex);
  thrift.write(m_check);
  thrift.write(m_clause);
  thrift.write(m_output);
  thrift.write(m_exceptionClass);
  thrift.write(m_exceptionObject);
}

void BreakPointInfo::recvImpl(DebuggerThriftBuffer &thrift) {
  TRACE(2, "BreakPointInfo::recvImpl\n");
  thrift.read(m_state);
  thrift.read(m_interrupt);
  thrift.read(m_file);
  thrift.read(m_line1);
  thrift.read(m_line2);
  thrift.read(m_namespace);
  thrift.read(m_class);
  thrift.read(m_funcs);
  thrift.read(m_url);
  thrift.read(m_regex);
  thrift.read(m_check);
  thrift.read(m_clause);
  thrift.read(m_output);
  thrift.read(m_exceptionClass);
  thrift.read(m_exceptionObject);
}

void BreakPointInfo::setClause(const std::string &clause, bool check) {
  TRACE(2, "BreakPointInfo::setClause\n");
  m_clause = clause;
  m_check = check;
}

void BreakPointInfo::changeBreakPointDepth(int stackDepth) {
  TRACE(2, "BreakPointInfo::changeBreakPointDepth\n");
  // if the breakpoint is equal or lower than the stack depth
  // delete it
  breakDepthStack.remove_if(
      std::bind2nd(std::greater_equal<int>(), stackDepth)
  );
}

void BreakPointInfo::unsetBreakable(int stackDepth) {
  TRACE(2, "BreakPointInfo::unsetBreakable\n");
  breakDepthStack.push_back(stackDepth);
}

void BreakPointInfo::setBreakable(int stackDepth) {
  TRACE(2, "BreakPointInfo::setBreakable\n");
  if (!breakDepthStack.empty() && breakDepthStack.back() == stackDepth) {
    breakDepthStack.pop_back();
  }
}

bool BreakPointInfo::breakable(int stackDepth) const {
  TRACE(2, "BreakPointInfo::breakable\n");
  if (!breakDepthStack.empty() && breakDepthStack.back() == stackDepth) {
    return false;
  } else {
    return true;
  }
}

void BreakPointInfo::toggle() {
  TRACE(2, "BreakPointInfo::toggle\n");
  switch (m_state) {
    case Always:   setState(Once);     break;
    case Once:     setState(Disabled); break;
    case Disabled: setState(Always);   break;
    default:
      assert(false);
      break;
  }
}

bool BreakPointInfo::valid() {
  TRACE(2, "BreakPointInfo::valid\n");
  if (m_valid) {
    switch (m_interrupt) {
      case BreakPointReached:
        if (!getFuncName().empty()) {
          if (!m_file.empty() || m_line1 != 0) {
            return false;
          }
        } else {
          if (m_file.empty() || m_line1 == 0) {
            return false;
          }
        }
        if (m_regex || m_funcs.size() > 1) {
          return false;
        }
        return (m_line1 && m_line2) || !m_file.empty() || !m_funcs.empty();
      case ExceptionThrown:
        return !m_class.empty();
      case RequestStarted:
      case RequestEnded:
      case PSPEnded:
        return true;
    }
  }
  return false;
}

bool BreakPointInfo::same(BreakPointInfoPtr bpi) {
  TRACE(2, "BreakPointInfo::same\n");
  return desc() == bpi->desc();
}

bool BreakPointInfo::match(InterruptType interrupt, InterruptSite &site) {
  TRACE(2, "BreakPointInfo::match\n");
  if (m_interrupt == interrupt) {
    switch (interrupt) {
      case RequestStarted:
      case RequestEnded:
      case PSPEnded:
        return
          checkUrl(site.url());
      case ExceptionThrown:
        return
          checkException(site.getException()) &&
          checkUrl(site.url()) && checkClause();
      case BreakPointReached:
      {
        bool match =
          Match(site.getFile(), site.getFileLen(), m_file, m_regex, false) &&
          checkLines(site.getLine0()) && checkStack(site) &&
          checkUrl(site.url()) && checkClause();

        if (!getFuncName().empty()) {
          // function entry breakpoint
          match = match && site.funcEntry();
        }
        return match;
      }
      default:
        break;
    }
  }
  return false;
}

std::string BreakPointInfo::state(bool padding) const {
  TRACE(2, "BreakPointInfo::state\n");
  switch (m_state) {
    case Always:   return padding ? "ALWAYS  " : "ALWAYS"  ;
    case Once:     return padding ? "ONCE    " : "ONCE"    ;
    case Disabled: return padding ? "DISABLED" : "DISABLED";
    default:
      assert(false);
      break;
  }
  return "";
}

std::string BreakPointInfo::regex(const std::string &name) const {
  TRACE(2, "BreakPointInfo::regex\n");
  if (m_regex) {
    return "regex{" + name + "}";
  }
  return name;
}

std::string BreakPointInfo::getNamespace() const {
  TRACE(2, "BreakPointInfo::getNamespace\n");
  if (!m_funcs.empty()) {
    return m_funcs[0]->m_namespace;
  }
  return "";
}

std::string BreakPointInfo::getClass() const {
  TRACE(2, "BreakPointInfo::getClass\n");
  if (!m_funcs.empty()) {
    return m_funcs[0]->m_class;
  }
  return "";
}

std::string BreakPointInfo::getFunction() const {
  TRACE(2, "BreakPointInfo::getFunction\n");
  if (!m_funcs.empty()) {
    return m_funcs[0]->m_function;
  }
  return "";
}

std::string BreakPointInfo::getFuncName() const {
  TRACE(2, "BreakPointInfo::getFuncName\n");
  if (!m_funcs.empty()) {
    return m_funcs[0]->getName();
  }
  return "";
}

std::string BreakPointInfo::site() const {
  TRACE(2, "BreakPointInfo::site\n");
  string ret;

  string preposition = "at ";
  if (!m_funcs.empty()) {
    ret = m_funcs[0]->site(preposition);
    for (unsigned int i = 1; i < m_funcs.size(); i++) {
      ret += " called by ";
      string tmp;
      ret += m_funcs[i]->site(tmp);
    }
  }

  if (!m_file.empty() || m_line1) {
    if (!ret.empty()) {
      ret += " ";
    } else {
      preposition = "";
    }
    if (m_line1) {
      ret += "on line " + lexical_cast<string>(m_line1);
      if (!m_file.empty()) {
        ret += " of " + m_file;
      }
    } else {
      ret += "in " + m_file;
    }
  }

  return preposition + ret;
}

std::string BreakPointInfo::descBreakPointReached() const {
  TRACE(2, "BreakPointInfo::descBreakPointReached\n");
  string ret;
  for (unsigned int i = 0; i < m_funcs.size(); i++) {
    ret += (i == 0 ? "upon entering " : " called by ");
    ret += m_funcs[i]->desc(this);
  }

  if (!m_file.empty() || m_line1 || m_line2) {
    if (!ret.empty()) {
      ret += " ";
    }
    if (m_line1 || m_line2) {
      if (m_line1 == m_line2) {
        ret += "on line " + lexical_cast<string>(m_line1);
      } else if (m_line2 == -1) {
        ret += "between line " + lexical_cast<string>(m_line1) + " and end";
      } else {
        ret += "between line " + lexical_cast<string>(m_line1) +
          " and line " + lexical_cast<string>(m_line2);
      }
      if (!m_file.empty()) {
        ret += " of " + regex(m_file);
      } else {
        ret += " of any file";
      }
    } else {
      ret += "on any lines in " + regex(m_file);
    }
  }
  return ret;
}

std::string BreakPointInfo::descExceptionThrown() const {
  TRACE(2, "BreakPointInfo::descExceptionThrown\n");
  string ret;
  if (!m_namespace.empty() || !m_class.empty()) {
    if (m_class == ErrorClassName) {
      ret = "right after an error";
    } else {
      ret = "right before throwing ";
      if (!m_class.empty()) {
        if (!m_namespace.empty()) {
          ret += regex(m_namespace) + "::";
        }
        ret += regex(m_class);
      } else {
        ret += "any exceptions in namespace " + regex(m_namespace);
      }
    }
  }
  return ret;
}

std::string BreakPointInfo::desc() const {
  TRACE(2, "BreakPointInfo::desc\n");
  string ret;
  switch (m_interrupt) {
    case BreakPointReached:
      ret = descBreakPointReached();
      break;
    case ExceptionThrown:
      ret = descExceptionThrown();
      break;
    default:
      ret = GetInterruptName((InterruptType)m_interrupt);
      break;
  }

  if (!m_url.empty()) {
    ret += " when request is " + regex(m_url);
  }

  if (!m_clause.empty()) {
    if (m_check) {
      ret += " if " + m_clause;
    } else {
      ret += " && " + m_clause;
    }
  }

  return ret;
}

bool BreakPointInfo::parseLines(const std::string &token) {
  TRACE(2, "BreakPointInfo::parseLines\n");
  if (token.empty()) return false;

  for (unsigned int i = 0; i < token.size(); i++) {
    char ch = token[i];
    if ((ch < '0' || ch > '9') && ch != '-') {
      return false;
    }
  }

  if (m_line1 || m_line2) {
    m_valid = false; // lines are specified twice
    return true;
  }

  vector<string> lines;
  Util::split('-', token.c_str(), lines);
  if (lines.empty() || lines.size() > 2 ||
      (lines.size() == 2 && lines[0].empty() && lines[1].empty())) {
    m_valid = false;
    return true;
  }

  if (lines[0].empty()) {
    m_line1 = 1;
  } else {
    m_line1 = atoi(lines[0].c_str());
    if (m_line1 <= 0) {
      m_valid = false;
      return true;
    }
  }
  if (lines.size() == 1) {
    m_line2 = m_line1;
    return true;
  }

  if (lines[1].empty()) {
    m_line2 = -1;
  } else {
    m_line2 = atoi(lines[1].c_str());
    if (m_line2 <= 0) {
      m_valid = false;
      return true;
    }
    if (m_line2 < m_line1) {
      int32_t tmp = m_line1;
      m_line1 = m_line2;
      m_line2 = tmp;
    }
  }

  return true;
}

void BreakPointInfo::parseBreakPointReached(const std::string &exp,
                                            const std::string &file) {
  TRACE(2, "BreakPointInfo::parseBreakPointReached\n");
  string input = exp;

  // everything after @ is URL
  size_t pos = input.find('@');
  if (pos != string::npos) {
    m_url = input.substr(pos + 1);
    if (pos == 0) return;
    input = input.substr(0, pos);
  }

  vector<string> tokens;
  bool seenClass = false;
  bool seenFile = false;
  Util::replaceAll(input, "=>", "=>:");
  Util::split(':', input.c_str(), tokens);
  for (unsigned int i = 0; i < tokens.size(); i++) {
    const std::string &token = tokens[i];
    if (parseLines(token)) continue;

    // if i'm ended with "::"
    if (i+1 < tokens.size() && tokens[i+1].empty()) {
      if (seenClass) {
        m_valid = false;
        return;
      }
      seenClass = true;
      if (i+3 < tokens.size() && tokens[i+3].empty()) {
        i += 2;
        m_namespace = token;
        m_class = tokens[i];
      } else {
        m_class = token;
      }

      i++;
      continue;
    }

    string ending;
    if (token.size() >= 2) {
      ending = token.substr(token.size() - 2);
    }
    if (token.size() >= 2 && (ending == "=>" || ending == "()")) {
      string func = token.substr(0, token.size() - 2);
      if (ending == "=>" &&
          func.size() >= 2 && func.substr(func.size() - 2) == "()") {
        func = func.substr(0, func.size() - 2);
      }
      if (token.empty()) {
        m_valid = false;
        return;
      }
      DFunctionInfoPtr pfunc(new DFunctionInfo());
      pfunc->m_namespace = m_namespace;
      pfunc->m_class = m_class;
      pfunc->m_function = func;
      m_funcs.insert(m_funcs.begin(), pfunc);
      m_namespace.clear();
      m_class.clear();
      seenClass = false;
    } else {
      if (!m_file.empty()) {
        m_valid = false;
        return;
      }
      if (seenFile) {
        m_valid = false;
        return;
      }
      seenFile = true;
      m_file = token;
    }
  }

  if (!m_namespace.empty() || !m_class.empty()) {
    DFunctionInfoPtr func(new DFunctionInfo());
    func->m_namespace = m_namespace;
    func->m_class = m_class;
    m_funcs.insert(m_funcs.begin(), func);
  }

  if (m_line1 && m_file.empty()) {
    m_file = file; // default to current file
  }
}

void BreakPointInfo::parseExceptionThrown(const std::string &exp) {
  TRACE(2, "BreakPointInfo::parseExceptionThrown\n");
  string input = exp;

  // everything after @ is URL
  size_t pos = input.find('@');
  if (pos != string::npos) {
    m_url = input.substr(pos + 1);
    input = input.substr(0, pos);
  }

  pos = input.find("::");
  if (pos != string::npos) {
    if (pos) {
      m_namespace = input.substr(0, pos);
    }
    m_class = input.substr(pos + 2);
  } else {
    m_class = input;
  }
  if (strncasecmp(m_class.c_str(), "error", m_class.size()) == 0) {
    m_class = ErrorClassName;
  }
}

bool BreakPointInfo::MatchFile(const char *haystack, int haystack_len,
                               const std::string &needle) {
  TRACE(2, "BreakPointInfo::MatchFile(const char *haystack\n");
  int pos = haystack_len - needle.size();
  if ((pos == 0 || haystack[pos - 1] == '/') &&
      strcasecmp(haystack + pos, needle.c_str()) == 0) {
    return true;
  }
  if (strcasecmp(StatCache::realpath(needle.c_str()).c_str(), haystack)
      == 0) {
    return true;
  }
  return false;
}

bool BreakPointInfo::MatchFile(const std::string& file,
                               const std::string& fullPath,
                               const std::string& relPath) {
  TRACE(2, "BreakPointInfo::MatchFile(const std::string&\n");
  if (file == fullPath || file == relPath) {
    return true;
  }
  if (file.find('/') == std::string::npos &&
      file == fullPath.substr(fullPath.rfind('/') + 1)) {
    return true;
  }
  // file is possibly setup with a symlink in the path
  if (StatCache::realpath(file.c_str()) ==
      StatCache::realpath(fullPath.c_str())) {
    return true;
  }
  return false;
}

bool BreakPointInfo::MatchClass(const char *fcls, const std::string &bcls,
                                bool regex, const char *func) {
  TRACE(2, "BreakPointInfo::MatchClass\n");
  if (bcls.empty()) return true;
  if (!fcls || !*fcls) return false;
  if (regex || !func || !*func) {
    return Match(fcls, 0, bcls, true, true);
  }

  StackStringData sdBClsName(bcls.c_str(), CopyString);
  VM::Class* clsB = VM::Unit::lookupClass(&sdBClsName);
  StackStringData sdFClsName(fcls, CopyString);
  VM::Class* clsF = VM::Unit::lookupClass(&sdFClsName);
  if (!clsB) return false;
  if (clsB == clsF) return true;
  StackStringData sdFuncName(func, CopyString);
  VM::Func* f = clsB->lookupMethod(&sdFuncName);
  if (!f) return false;
  return (f->baseCls() == clsF);
}

bool BreakPointInfo::Match(const char *haystack, int haystack_len,
                           const std::string &needle, bool regex, bool exact) {
  TRACE(2, "BreakPointInfo::Match\n");
  if (needle.empty()) {
    return true;
  }
  if (!haystack || !*haystack) {
    return false;
  }

  if (!regex) {
    if (exact) {
      return strcasecmp(haystack, needle.c_str()) == 0;
    }
    return MatchFile(haystack, haystack_len, needle);
  }

  Variant matches;
  Variant r = preg_match(String(needle.c_str(), needle.size(),
                                CopyString),
                         String(haystack, haystack_len, CopyString),
                         matches);
  return r.same(1);
}

bool BreakPointInfo::checkException(CVarRef e) {
  TRACE(2, "BreakPointInfo::checkException\n");
  assert(!e.isNull());
  if (e.isObject()) {
    if (m_regex) {
      return Match(m_class.c_str(), m_class.size(),
                   e.toObject()->o_getClassName().data(), true, false);
    }
    return e.instanceof(m_class.c_str());
  }
  return Match(m_class.c_str(), m_class.size(), ErrorClassName, m_regex,
               false);
}

bool BreakPointInfo::checkUrl(std::string &url) {
  TRACE(2, "BreakPointInfo::checkUrl\n");
  if (!m_url.empty()) {
    if (url.empty()) {
      url = "/";
      Transport *transport = g_context->getTransport();
      if (transport) {
        url += transport->getCommand();
      }
    }
    return Match(url.c_str(), url.size(), m_url, m_regex, false);
  }
  return true;
}

bool BreakPointInfo::checkLines(int line) {
  TRACE(2, "BreakPointInfo::checkLines\n");
  if (m_line1) {
    assert(m_line2 == -1 || m_line2 >= m_line1);
    return line >= m_line1 && (m_line2 == -1 || line <= m_line2);
  }
  return true;
}

bool BreakPointInfo::checkStack(InterruptSite &site) {
  TRACE(2, "BreakPointInfo::checkStack\n");
  if (m_funcs.empty()) return true;

  if (!Match(site.getNamespace(), 0, m_funcs[0]->m_namespace, m_regex, true) ||
      !Match(site.getFunction(),  0, m_funcs[0]->m_function,  m_regex, true) ||
      !MatchClass(site.getClass(), m_funcs[0]->m_class, m_regex,
                  site.getFunction())) {
    return false;
  }

  return true;
}

bool BreakPointInfo::checkClause() {
  TRACE(2, "BreakPointInfo::checkClause\n");
  if (!m_clause.empty()) {
    if (m_php.empty()) {
      if (m_check) {
        m_php = DebuggerProxy::MakePHPReturn(m_clause);
      } else {
        m_php = DebuggerProxy::MakePHP(m_clause);
      }
    }
    String output;
    {
      EvalBreakControl eval(false);
      Variant ret = DebuggerProxy::ExecutePHP(m_php, output, false, 0);
      if (m_check) {
        return ret.toBoolean();
      }
    }
    m_output = std::string(output.data(), output.size());
    return true;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

void BreakPointInfo::SendImpl(const BreakPointInfoPtrVec &bps,
                              DebuggerThriftBuffer &thrift) {
  TRACE(2, "BreakPointInfo::SendImpl\n");
  int16_t size = bps.size();
  thrift.write(size);
  for (int i = 0; i < size; i++) {
    bps[i]->sendImpl(thrift);
  }
}

void BreakPointInfo::RecvImpl(BreakPointInfoPtrVec &bps,
                              DebuggerThriftBuffer &thrift) {
  TRACE(2, "BreakPointInfo::RecvImpl\n");
  int16_t size;
  thrift.read(size);
  bps.resize(size);
  for (int i = 0; i < size; i++) {
    BreakPointInfoPtr bpi(new BreakPointInfo());
    bpi->recvImpl(thrift);
    bps[i] = bpi;
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
