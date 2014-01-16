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

#include "hphp/util/text-util.h"

#include <string>
#include <vector>

namespace HPHP {

using std::string;
using std::vector;

// --- Static functions.

bool TextUtil::BeginsWith(const string& str1, const string& str2) {
  // | [----------str2--------]
  // | <<<<<<<<< match >>>>>>>>
  // | [--------------------------------str1-------]

  if (str2.length() > str1.length()) {
    return false;
  }

  return str1.find(str2) == 0;
}

string TextUtil::StripCommonStart(const string& str1, const string& str2) {
  if (!BeginsWith(str1, str2)) {
    return str1;
  }

  return str1.substr(str2.length());
}

bool TextUtil::EndsWith(const string& str, char ch) {
  return *str.rbegin() == ch;
}

string TextUtil::StripTrailing(const std::string& str, char ch) {
  string temp(str);

  size_t i = str.find_last_not_of(ch);

  if (i != string::npos) {
    temp.erase(i + 1);
  } else {
    temp.clear();     // Nothing but those chars.
  }

  return temp;
}

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

}  // namespace HPHP
