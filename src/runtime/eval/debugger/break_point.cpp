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

using namespace std;
using namespace boost;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

const char *InterruptSite::getFile() const {
  if (m_file.isNull()) {
    m_file = m_frame->getFileName();
  }
  return m_file.data();
}

const char *InterruptSite::getFunction() const {
  if (m_function == NULL) {
    m_function = m_frame->getFunction();
    if (m_frame->getFlags() & FrameInjection::PseudoMain) {
      m_function = "";
    }
    const char *name = strstr(m_function, "::");
    if (name) {
      m_function = name + 2;
    }
  }
  return m_function;
}

int InterruptSite::getFileLen() const {
  if (m_file_strlen == -1) {
    m_file_strlen = strlen(getFile());
  }
  return m_file_strlen;
}

std::string InterruptSite::desc() const {
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

///////////////////////////////////////////////////////////////////////////////

const char *BreakPointInfo::ErrorClassName = "@";

const char *BreakPointInfo::GetInterruptName(InterruptType interrupt) {
  switch (interrupt) {
    case RequestStarted: return "start of request";
    case RequestEnded:   return "end of request or start of psp";
    case PSPEnded:       return "end of psp";
    default:
      ASSERT(false);
      break;
  }
  return NULL;
}

BreakPointInfo::BreakPointInfo(bool regex, State state,
                               const std::string &file, int line)
    : m_index(0), m_state(state), m_valid(true),
      m_interrupt(BreakPointReached),
      m_file(file), m_line1(line), m_line2(line),
      m_regex(regex), m_check(false) {
  createIndex();
}

BreakPointInfo::BreakPointInfo(bool regex, State state,
                               InterruptType interrupt,
                               const std::string &url)
    : m_index(0), m_state(state), m_valid(true), m_interrupt(interrupt),
      m_line1(0), m_line2(0), m_url(url),
      m_regex(regex), m_check(false) {
  createIndex();
}

BreakPointInfo::BreakPointInfo(bool regex, State state,
                               InterruptType interrupt,
                               const std::string &exp,
                               const std::string &file)
    : m_index(0), m_state(state), m_valid(true), m_interrupt(interrupt),
      m_line1(0), m_line2(0),
      m_regex(regex), m_check(false) {
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
  m_index = ++s_max_breakpoint_index;
}

BreakPointInfo::~BreakPointInfo() {
  if (m_index && m_index == s_max_breakpoint_index) {
    --s_max_breakpoint_index;
  }
}

void BreakPointInfo::sendImpl(DebuggerThriftBuffer &thrift) {
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
  m_clause = clause;
  m_check = check;
}

void BreakPointInfo::toggle() {
  switch (m_state) {
    case Always:   m_state = Once;     break;
    case Once:     m_state = Disabled; break;
    case Disabled: m_state = Always;   break;
    default:
      ASSERT(false);
      break;
  }
}

bool BreakPointInfo::valid() {
  if (m_valid) {
    switch (m_interrupt) {
      case BreakPointReached:
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
  return desc() == bpi->desc();
}

bool BreakPointInfo::match(InterruptType interrupt, InterruptSite &site) {
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
        return
          Match(site.getFile(), site.getFileLen(), m_file, m_regex, false) &&
          checkLines(site.getLine0()) && checkStack(site) &&
          checkUrl(site.url()) && checkFrame(site.getFrame()) && checkClause();
      default:
        break;
    }
  }
  return false;
}

std::string BreakPointInfo::state(bool padding) const {
  switch (m_state) {
    case Always:   return padding ? "ALWAYS  " : "ALWAYS"  ;
    case Once:     return padding ? "ONCE    " : "ONCE"    ;
    case Disabled: return padding ? "DISABLED" : "DISABLED";
    default:
      ASSERT(false);
      break;
  }
  return "";
}

std::string BreakPointInfo::regex(const std::string &name) const {
  if (m_regex) {
    return "regex{" + name + "}";
  }
  return name;
}

std::string BreakPointInfo::getNamespace() const {
  if (!m_funcs.empty()) {
    return m_funcs[0]->m_namespace;
  }
  return "";
}

std::string BreakPointInfo::getClass() const {
  if (!m_funcs.empty()) {
    return m_funcs[0]->m_class;
  }
  return "";
}

std::string BreakPointInfo::getFunction() const {
  if (!m_funcs.empty()) {
    return m_funcs[0]->m_function;
  }
  return "";
}

std::string BreakPointInfo::site() const {
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
      int32 tmp = m_line1;
      m_line1 = m_line2;
      m_line2 = tmp;
    }
  }

  return true;
}

void BreakPointInfo::parseBreakPointReached(const std::string &exp,
                                            const std::string &file) {
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
  int pos = haystack_len - needle.size();
  return (pos == 0 || haystack[pos - 1] == '/') &&
    strcasecmp(haystack + pos, needle.c_str()) == 0;
}

bool BreakPointInfo::MatchClass(const char *fcls, const std::string &bcls,
                                bool regex, const char *func) {
  if (bcls.empty()) return true;
  if (!fcls || !*fcls) return false;
  if (regex || !func || !*func) {
    return Match(fcls, 0, bcls, true, true);
  }
  if (strcasecmp(fcls, bcls.c_str()) == 0) {
    return true;
  }

  const ClassInfo *clsInfo = ClassInfo::FindClass(bcls.c_str());
  if (clsInfo) {
    ClassInfo *foundClass;
    if (clsInfo->hasMethod(func, foundClass) && foundClass) {
      const char *name = foundClass->getName();
      return strcasecmp(fcls, name) == 0;
    }
  }
  return false;
}

bool BreakPointInfo::Match(const char *haystack, int haystack_len,
                           const std::string &needle, bool regex, bool exact) {
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
                                AttachLiteral),
                         String(haystack, haystack_len, AttachLiteral),
                         matches);
  return r.same(1);
}

bool BreakPointInfo::checkException(CVarRef e) {
  ASSERT(!e.isNull());
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
  if (m_line1) {
    ASSERT(m_line2 == -1 || m_line2 >= m_line1);
    return line >= m_line1 && (m_line2 == -1 || line <= m_line2);
  }
  return true;
}

bool BreakPointInfo::checkStack(InterruptSite &site) {
  if (m_funcs.empty()) return true;

  if (!Match(site.getNamespace(), 0, m_funcs[0]->m_namespace, m_regex, true) ||
      !Match(site.getFunction(),  0, m_funcs[0]->m_function,  m_regex, true) ||
      !MatchClass(site.getClass(), m_funcs[0]->m_class, m_regex,
                  site.getFunction())) {
    return false;
  }

  FrameInjection *f = site.getFrame()->getPrev();
  for (unsigned int i = 1; i < m_funcs.size(); i++) {
    for (; f; f = f->getPrev()) {
      InterruptSite prevSite(f);
      if (Match(prevSite.getNamespace(), 0,
                m_funcs[i]->m_namespace, m_regex, true) &&
          Match(prevSite.getFunction(), 0,
                m_funcs[i]->m_function, m_regex, true) &&
          MatchClass(prevSite.getClass(), m_funcs[i]->m_class, m_regex,
                     site.getFunction())) {
        break;
      }
    }
    if (f == NULL) {
      return false;
    }
  }
  return true;
}

bool BreakPointInfo::checkFrame(FrameInjection *frame) {
  // If we're not specifying a breakpoint's line number, we only break just
  // once per frame.
  if (!m_line1) {
    return (frame->getFlags() & FrameInjection::BreakPointHit) == 0;
  }
  return true;
}

bool BreakPointInfo::checkClause() {
  if (!m_clause.empty()) {
    if (m_php.empty()) {
      if (m_check) {
        m_php = DebuggerProxy::MakePHPReturn(m_clause);
      } else {
        m_php = DebuggerProxy::MakePHP(m_clause);
      }
    }
    String output;
    Variant ret = DebuggerProxy::ExecutePHP(m_php, output, false, 0);
    if (m_check) {
      return ret.toBoolean();
    }
    m_output = std::string(output.data(), output.size());
    return true;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

void BreakPointInfo::SendImpl(const BreakPointInfoPtrVec &bps,
                              DebuggerThriftBuffer &thrift) {
  int16 size = bps.size();
  thrift.write(size);
  for (int i = 0; i < size; i++) {
    bps[i]->sendImpl(thrift);
  }
}

void BreakPointInfo::RecvImpl(BreakPointInfoPtrVec &bps,
                              DebuggerThriftBuffer &thrift) {
  int16 size;
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
