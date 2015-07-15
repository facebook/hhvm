/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/text-util.h"

#include <cassert>
#include <cstring> // memcpy
#include <string>
#include <vector>
#include "hphp/util/string-vsnprintf.h"

namespace HPHP {

using std::string;
using std::vector;

// --- Static functions.

vector<string> TextUtil::MakePathList(const string& path) {
  vector<string> temp;

  if (path.empty()) {
    return temp;
  }

  size_t start = 0;
  for (size_t i = 1; i < path.length(); ++i) {
    if (path[i] == '/') {
      temp.push_back(path.substr(start, i));
    }
  }

  return temp;
}

void split(char delimiter, const char *s, vector<string> &out,
           bool ignoreEmpty /* = false */) {
  assert(s);

  const char *start = s;
  const char *p = s;
  for (; *p; p++) {
    if (*p == delimiter) {
      if (!ignoreEmpty || p > start) {
        out.push_back(string(start, p - start));
      }
      start = p + 1;
    }
  }
  if (!ignoreEmpty || p > start) {
    out.push_back(string(start, p - start));
  }
}

void replaceAll(string &s, const char *from, const char *to) {
  assert(from && *from);
  assert(to);

  string::size_type lenFrom = strlen(from);
  string::size_type lenTo = strlen(to);
  for (string::size_type pos = s.find(from);
       pos != string::npos;
       pos = s.find(from, pos + lenTo)) {
    s.replace(pos, lenFrom, to);
  }
}

std::string toLower(const std::string &s) {
  unsigned int len = s.size();
  string ret;
  if (len) {
    ret.reserve(len);
    for (unsigned int i = 0; i < len; i++) {
      ret += tolower(s[i]);
    }
  }
  return ret;
}

std::string toUpper(const std::string &s) {
  unsigned int len = s.size();
  string ret;
  ret.reserve(len);
  for (unsigned int i = 0; i < len; i++) {
    ret += toupper(s[i]);
  }
  return ret;
}

std::string getIdentifier(const std::string &fileName) {
  string ret = "hphp_" + fileName;
  replaceAll(ret, "/", "__");
  replaceAll(ret, ".", "__");
  replaceAll(ret, "-", "__");
  return ret;
}

std::string escapeStringForCPP(const char *input, int len,
                               bool* binary /* = NULL */) {
  if (binary) *binary = false;
  string ret;
  ret.reserve((len << 1) + 2);
  for (int i = 0; i < len; i++) {
    unsigned char ch = input[i];
    switch (ch) {
      case '\n': ret += "\\n";  break;
      case '\r': ret += "\\r";  break;
      case '\t': ret += "\\t";  break;
      case '\a': ret += "\\a";  break;
      case '\b': ret += "\\b";  break;
      case '\f': ret += "\\f";  break;
      case '\v': ret += "\\v";  break;
      case '\0': ret += "\\000"; if (binary) *binary = true; break;
      case '\"': ret += "\\\""; break;
      case '\\': ret += "\\\\"; break;
      case '?':  ret += "\\?";  break; // avoiding trigraph errors
      default:
        if (isprint(ch)) {
          ret += ch;
        } else {
          // output in octal notation
          char buf[10];
          snprintf(buf, sizeof(buf), "\\%03o", ch);
          ret += buf;
        }
        break;
    }
  }
  return ret;
}

std::string escapeStringForPHP(const char *input, int len) {
  string output;
  output.reserve((len << 1) + 2);
  output = "'";
  for (int i = 0; i < len; i++) {
    unsigned char ch = input[i];
    switch (ch) {
    case '\n': output += "'.\"\\n\".'";  break;
    case '\r': output += "'.\"\\r\".'";  break;
    case '\t': output += "'.\"\\t\".'";  break;
    case '\'': output += "'.\"'\".'";    break;
    case '\\': output += "'.\"\\\\\".'"; break;
    case '\0': output += "'.\"\\0\".'";  break;
    default:
      output += ch;
      break;
    }
  }
  output += "'";
  replaceAll(output, ".''.", ".");
  replaceAll(output, "''.", "");
  replaceAll(output, ".''", "");
  replaceAll(output, "\".\"", "");
  return output;
}

const void *buffer_duplicate(const void *src, int size) {
  char *s = (char *)malloc(size + 1); // '\0' in the end
  memcpy(s, src, size);
  s[size] = '\0';
  return s;
}

const void *buffer_append(const void *buf1, int size1,
                          const void *buf2, int size2) {
  char *s = (char *)realloc(const_cast<void *>(buf1), size1 + size2 + 1);
  memcpy((char *)s + size1, buf2, size2);
  s[size1 + size2] = '\0';
  return s;
}

void string_printf(std::string &msg, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
}

std::string format_pattern(const std::string &pattern, bool prefixSlash) {
  if (pattern.empty()) return pattern;

  std::string ret = "#";
  for (unsigned int i = 0; i < pattern.size(); i++) {
    char ch = pattern[i];

    // apache rewrite rules don't require initial slash
    if (prefixSlash && i == 0 && ch == '^') {
      char ch1 = pattern[1];
      if (ch1 != '/' && ch1 != '(') {
        ret += "^/";
        continue;
      }
    }

    if (ch == '#') {
      ret += "\\#";
    } else {
      ret += ch;
    }
  }
  ret += '#';
  return ret;
}

}  // namespace HPHP
