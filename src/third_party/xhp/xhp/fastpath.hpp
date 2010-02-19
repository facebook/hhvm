/*
  +----------------------------------------------------------------------+
  | XHP                                                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 2009 - 2010 Facebook, Inc. (http://www.facebook.com)          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE.PHP, and is    |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
*/

#pragma once
#include "xhp_preprocess.hpp"

/**
 * Provides a fast way to scan over a buffer and figure out whether or not it
 * contains code that XHP needs to rewrite. This is valuable because the full
 * XHP parser is a lot more expensive to run. Since we assume (for now) that
 * most files won't contain XHP this fast fail greatly speeds up overal parse
 * times.
 *
 * The nature of this function is that it still may return false positives since
 * it only runs a simple lexical analysis on the source text. Particularly,
 * ternary operators returning numbers or constants with no spaces in between
 * tokens can fool it. i.e. `$foo = true ? 1:2` or `$foo = true ? foo:bar`.
 * That's not to say you should change your coding conventions because of this,
 * since it has no side effect other than slightly-slower parsing. Instead you
 * should install APC or something.
 *
 * Note that at this point the `len` parameter is just a formality and `yy` MUST
 * be NULL terminated. re2c doesn't provide any dependable means to scan a
 * buffer with no sentinal value. So for instance, if you want to use this on an
 * mmap'd file you must read it into memory first.
 *
 * Furthermore, it seems the primary maintainer of re2c doesn't want to
 * implement this, so we have to work around it for now.
 *
 * See: re2c-Support Requests-2407218 --
 * http://sourceforge.net/tracker/?func=detail&atid=616201&aid=2407218&group_id=96864
 */
bool xhp_fastpath(const char* yy, const size_t len, const xhp_flags_t &flags);
