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

#ifndef incl_HPHP_JIT_STRING_TAG_H_
#define incl_HPHP_JIT_STRING_TAG_H_

#include <cstdint>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * A compact identifier corresponding to a static string label.
 *
 * Unlike static StringData*'s, these aren't used by jitted code or the
 * runtime, and are derived from a small set of string literals.
 *
 * Guaranteed to be "invalid" when default-initialized.
 */
using StringTag = uint8_t;

/*
 * Encode or decode a StringTag.
 */
StringTag tag_from_string(const char*);
const char* string_from_tag(StringTag);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
