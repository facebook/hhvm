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

#ifndef __HPHP_TAINT_HELPER_H__
#define __HPHP_TAINT_HELPER_H__

#ifdef TAINTED

namespace HPHP {
void taint_warn_if_tainted(CStrRef s, const bitstring bit);
void taint_array_variant(Variant& v, const std::string original_str);
}

#endif // TAINTED

#endif // __HPHP_TAINT_HELPER_H__

