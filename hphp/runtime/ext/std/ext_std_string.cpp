/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/std/ext_std_string.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/zend-printf.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_nl("\n");

Variant HHVM_FUNCTION(wordwrap, const String& str, int64_t linewidth /* = 75 */,
                      const String& brk /* = s_nl */, bool cut /* = false */) {
  const char* brkstr = brk.data();
  size_t textlen = str.size();
  size_t brklen = brk.size();

  if (textlen == 0) {
    return empty_string();
  }
  if (brklen == 0) {
    raise_warning("Break string cannot be empty");
    return false;
  }
  if (linewidth == 0 && cut) {
    raise_warning("Can't force cut when width is zero");
    return false;
  }
  size_t w = linewidth >= 0 ? linewidth : 0;

  // If the string's length is less than or equal to the specified
  // width, there's nothing to do and we can just return the string.
  if (textlen <= w) return str;

  // Special case for a single-character break as it needs no
  // additional storage space
  if (brklen == 1 && !cut) {
    auto new_sd = StringData::Make(str.get(), CopyString);
    new_sd->invalidateHash();
    auto ret = Variant::attach(new_sd);
    char* newtext = new_sd->mutableData();
    auto bc = brkstr[0];
    size_t current = 0, laststart = 0, lastspace = 0;
    for (; current < textlen; current++) {
      if (newtext[current] == bc) {
        laststart = lastspace = current + 1;
      } else if (newtext[current] == ' ') {
        if (current - laststart >= w) {
          newtext[current] = bc;
          laststart = current + 1;
        }
        lastspace = current;
      } else if (current - laststart >= w && laststart != lastspace) {
        newtext[lastspace] = bc;
        laststart = lastspace + 1;
      }
    }
    return ret;
  }

  // Multiple character line break or forced cut

  // Estimate how big the output string will be. It's okay if this estimate
  // is wrong as we will grow or shrink as needed. The goals here are two-
  // fold: (1) avoid the need to grow or shrink in the common case, and
  // (2) for extreme cases where it's hard to make an accurate estimate
  // (ex. when w is very small or brk is very large) we should be careful
  // to avoid making huge over-estimations.
  StringBuffer strbuf(
    textlen +
    textlen / (std::max<size_t>(w, 16) - 8) * std::min<size_t>(brklen, 8));

  const char* text = str.data();
  size_t current = 0, laststart = 0, lastspace = 0;
  for (; current < textlen; current++) {
    // when we hit an existing break, copy to new buffer, and
    // fix up laststart and lastspace
    if (text[current] == brkstr[0] && current + brklen < textlen &&
        !strncmp(text + current, brkstr, brklen)) {
      strbuf.append(text + laststart, current - laststart + brklen);
      current += brklen - 1;
      laststart = lastspace = current + 1;
    }
    // if it is a space, check if it is at the line boundary,
    // copy and insert a break, or just keep track of it
    else if (text[current] == ' ') {
      if (current - laststart >= w) {
        strbuf.append(text + laststart, current - laststart);
        strbuf.append(brkstr, brklen);
        laststart = current + 1;
      }
      lastspace = current;
    }
    // if we are cutting, and we've accumulated enough
    // characters, and we haven't see a space for this line,
    // copy and insert a break.
    else if (current - laststart >= w && cut && laststart >= lastspace) {
      strbuf.append(text + laststart, current - laststart);
      strbuf.append(brkstr, brklen);
      laststart = lastspace = current;
    }
    // if the current word puts us over width w, copy back up
    // until the last space, insert a break, and move up the
    // laststart
    else if (current - laststart >= w && laststart < lastspace) {
      strbuf.append(text + laststart, lastspace - laststart);
      strbuf.append(brkstr, brklen);
      laststart = lastspace = lastspace + 1;
    }
  }
  // copy over any stragglers
  if (laststart != current) {
    strbuf.append(text + laststart, current - laststart);
  }

  auto s = strbuf.detach();

  // if it's not possible to reduce the output string's capacity by more
  // than 25%, then we can just return the string as is.
  size_t estShrinkCap =
    MemoryManager::estimateCap(sizeof(StringData) + s.size() + 1);
  if (estShrinkCap * 4 >= (size_t)s.capacity() * 3) {
    return s;
  }
  // reallocate into a smaller buffer so that we don't waste memory
  return Variant::attach(StringData::Make(s.get(), CopyString));
}

///////////////////////////////////////////////////////////////////////////////

// We take format as a variant in order to get the right conversion
// failure notice (as opposed to parameter typehint warning)
Variant HHVM_FUNCTION(sprintf, const Variant& vformat, const Array& args) {
  String format = vformat.toString();
  String output = string_printf(format.data(), format.size(), args);
  if (output.isNull()) return false;
  return output;
}

Variant HHVM_FUNCTION(vsprintf, const Variant& vformat, const Variant& args) {
  String format = vformat.toString();
  String output = string_printf(format.data(), format.size(), args.toArray());
  if (output.isNull()) return false;
  return output;
}

Variant HHVM_FUNCTION(printf, const Variant& vformat, const Array& args) {
  String format = vformat.toString();
  String output = string_printf(format.data(), format.size(), args);
  if (output.isNull()) return false;
  g_context->write(output.data(), output.size());
  return output.size();
}

Variant HHVM_FUNCTION(vprintf, const Variant& vformat, const Variant& args) {
  String format = vformat.toString();
  String output = string_printf(format.data(), format.size(), args.toArray());
  if (output.isNull()) return false;
  g_context->write(output.data(), output.size());
  return output.size();
}

///////////////////////////////////////////////////////////////////////////////

void StandardExtension::initString() {
  HHVM_FE(wordwrap);

  HHVM_FE(sprintf);
  HHVM_FE(vsprintf);
  HHVM_FE(printf);
  HHVM_FE(vprintf);

  loadSystemlib("std_string");
}

}
