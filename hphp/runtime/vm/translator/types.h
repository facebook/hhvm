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

#ifndef incl_TRANSL_TYPES_H_
#define incl_TRANSL_TYPES_H_

#include "util/base.h"

namespace HPHP {
namespace VM {
namespace Transl {

/*
 * Core types.
 */
typedef unsigned char* TCA; // "Translation cache adddress."
typedef const unsigned char* CTCA;

struct ctca_identity_hash {
  size_t operator()(CTCA val) const {
    // Experiments show that this is a sufficient "hash function" on
    // TCAs for now; using stronger functions didn't help given current
    // data. Patterns of code emission in the translator could invalidate
    // this finding going forward, though; e.g., if we frequently emit
    // a call instruction N bytes into a cache-aligned region.
    return uintptr_t(val);
  }
};


typedef uint32_t               TransID;
typedef hphp_hash_set<TransID> TransIDSet;

}}}

#endif
