/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/string-tag.h"

#include "hphp/util/assertions.h"
#include "hphp/util/lock.h"
#include "hphp/util/mutex.h"
#include "hphp/util/safe-cast.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

ReadWriteMutex s_mutex;

std::unordered_map<std::string,StringTag> s_string_to_tag;
std::vector<const char*> s_tag_to_string;

}

StringTag tag_from_string(const char* str) {
  { // Return the tag if we've already cached this string.
    ReadLock l(s_mutex, false);
    auto const it = s_string_to_tag.find(str);
    if (it != s_string_to_tag.end()) return it->second;
  }
  { // Check again with the write lock held.
    WriteLock l(s_mutex, false);
    auto const it = s_string_to_tag.find(str);
    if (it != s_string_to_tag.end()) return it->second;

    s_tag_to_string.push_back(str);

    // The tag is the one-indexed index into the vector.
    auto const tag = safe_cast<StringTag>(s_tag_to_string.size());
    s_string_to_tag[str] = tag;

    return tag;
  }
}

const char* string_from_tag(StringTag tag) {
  if (tag == 0) return nullptr;

  ReadLock l(s_mutex, false);
  assertx(tag <= s_tag_to_string.size());
  return s_tag_to_string[tag - 1];
}

///////////////////////////////////////////////////////////////////////////////

}}
