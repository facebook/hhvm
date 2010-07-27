/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <runtime/eval/debugger/debugger_thrift_buffer.h>
#include <runtime/eval/eval.h>
#include <runtime/base/preg.h>
#include <runtime/base/externals.h>

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

///////////////////////////////////////////////////////////////////////////////

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
    : m_state(state), m_valid(true), m_interrupt(BreakPointReached),
      m_file(file), m_line1(line), m_line2(line),
      m_regex(regex), m_check(false) {
}

BreakPointInfo::BreakPointInfo(bool regex, State state,
                               InterruptType interrupt,
                               const std::string &url)
    : m_state(state), m_valid(true), m_interrupt(interrupt),
      m_line1(0), m_line2(0), m_url(url),
      m_regex(regex), m_check(false) {
}

BreakPointInfo::BreakPointInfo(bool regex, State state,
                               InterruptType interrupt,
                               const std::string &exp,
                               const std::string &file)
    : m_state(state), m_valid(true), m_interrupt(interrupt),
      m_line1(0), m_line2(0),
      m_regex(regex), m_check(false) {
  if (m_interrupt == ExceptionThrown) {
    parseExceptionThrown(exp);
    if (!m_file.empty() || m_line1 || m_line2 || !m_function.empty()) {
      m_valid = false;
    }
  } else {
    parseBreakPointReached(exp, file);
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
  thrift.write(m_function);
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
  thrift.read(m_function);
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
  return m_valid &&
    ((m_line1 && m_line2) || !m_file.empty() ||
     !m_function.empty() || !m_class.empty() || !m_namespace.empty() ||
     (m_interrupt != BreakPointReached && m_interrupt != ExceptionThrown));
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
          checkLines(site.getLine()) &&
          Match(site.getNamespace(), 0, m_namespace, m_regex, true) &&
          Match(site.getClass(), 0, m_class, m_regex, true) &&
          Match(site.getFunction(), 0, m_function, m_regex, true) &&
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

std::string BreakPointInfo::site() const {
  string ret;
  string preposition = "at ";
  if (!m_namespace.empty() || !m_class.empty() || !m_function.empty()) {
    if (!m_class.empty()) {
      if (!m_namespace.empty()) {
        ret = m_namespace + "::";
      }
      ret += m_class;
      if (!m_function.empty()) {
        ret += "::" + m_function + "()";
      } else {
        ret = "class " + ret;
        preposition = "in ";
      }
    } else {
      if (!m_function.empty()) {
        ret = m_function + "()";
        if (!m_namespace.empty()) {
          ret += " in namespace " + m_namespace;
        }
      } else if (!m_namespace.empty()) {
        ret = "namespace " + m_namespace;
        preposition = "in ";
      }
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
  if (!m_namespace.empty() || !m_class.empty() || !m_function.empty()) {
    ret = "upon entering ";
    if (!m_class.empty()) {
      string cls;
      if (!m_namespace.empty()) {
        cls = regex(m_namespace) + "::";
      }
      cls += regex(m_class);
      if (!m_function.empty()) {
        ret += cls + "::" + regex(m_function) + "()";
      } else {
        ret += "any functions in class " + cls;
      }
    } else {
      if (!m_function.empty()) {
        ret += regex(m_function) + "()";
        if (!m_namespace.empty()) {
          ret += " in namespace " + regex(m_namespace);
        }
      } else {
        ret += "any functions in namespace " + regex(m_namespace);
      }
    }
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
    input = input.substr(0, pos);
  }

  vector<string> tokens;
  bool seenClass = false;
  bool seenFile = false;
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

    if (token.size() >= 2 && token.substr(token.size() - 2) == "()") {
      if (token.empty() || !m_function.empty()) {
        m_valid = false;
        return;
      }
      m_function = token.substr(0, token.size() - 2);
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
    m_namespace = input.substr(0, pos);
    m_class = input.substr(pos + 2);
  } else {
    m_class = input;
  }
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
    int pos = haystack_len - needle.size();
    return (pos == 0 || haystack[pos - 1] == '/') &&
      strcasecmp(haystack + pos, needle.c_str()) == 0;
  }

  Variant matches;
  Variant r = preg_match(String(needle.c_str(), needle.size(),
                                AttachLiteral),
                         String(haystack, haystack_len, AttachLiteral),
                         matches);
  return r.same(1);
}

bool BreakPointInfo::checkException(CObjRef e) {
  ASSERT(!e.isNull());
  if (m_regex) {
    return Match(m_class.c_str(), m_class.size(), e->o_getClassName(),
                 true, false);
  }
  return e.instanceof(m_class.c_str());
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
    PSEUDOMAIN_INJECTION(_); // using "_" as filename
    String output;
    Variant ret;
    try {
      g_context->obStart("");
      if (m_php.empty()) {
        if (m_check) {
          m_php = "<?php return (" + m_clause + ");";
        } else {
          m_php = "<?php " + m_clause + ";";
        }
      }
      ret = eval(get_variable_table(), Object(),
                 String(m_php.c_str(), m_php.size(), AttachLiteral), false);

      output = Debugger::ColorStdout(g_context->obDetachContents());
      g_context->obClean();
      g_context->obEnd();
    } catch (Exception &e) {
      output = Debugger::ColorStderr(String(e.what()));
    }

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
