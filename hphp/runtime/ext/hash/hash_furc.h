/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_HASH_FURC_H_
#define incl_HPHP_EXT_HASH_FURC_H_

#include <stdint.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

uint32_t furc_hash_internal(const char* const key, const size_t len, const uint32_t m);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_HASH_FURC_H_


