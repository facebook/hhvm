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

// Magic numbers associated with this cache.

#ifndef incl_HPHP_MAGIC_NUMBERS_H_
#define incl_HPHP_MAGIC_NUMBERS_H_

namespace HPHP {

const uint64_t kCacheFileMagic = 0x55aa0cb0cefa4b52;
const uint64_t kDirectoryTerminatorMagic = 0x5228bb574d524554;

// This one should only be on disk during the write process.
// See cache-saver.h for a discussion of the "surgical updates".
const uint64_t kDataOfsPlaceholder = 0x000044448888aaaa;

}  // namespace HPHP

#endif  // incl_HPHP_MAGIC_NUMBERS_H_
