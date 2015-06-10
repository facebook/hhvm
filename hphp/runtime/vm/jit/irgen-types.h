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
#ifndef incl_HPHP_JIT_IRGEN_TYPES_H_
#define incl_HPHP_JIT_IRGEN_TYPES_H_

#include <folly/Optional.h>

namespace HPHP {
struct RepoAuthType;
struct StringData;
}
namespace HPHP { namespace jit {
struct SSATmp;
struct IRGS;
struct Type;
}}

namespace HPHP { namespace jit { namespace irgen {

//////////////////////////////////////////////////////////////////////

folly::Optional<Type> ratToAssertType(IRGS& env, RepoAuthType rat);
SSATmp* implInstanceOfD(IRGS& env, SSATmp* src, const StringData* className);

//////////////////////////////////////////////////////////////////////

}}}

#endif
