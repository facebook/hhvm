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
#ifndef incl_HPHP_VM_REQUEST_ARENA_H_
#define incl_HPHP_VM_REQUEST_ARENA_H_

#include <boost/aligned_storage.hpp>

#include "hphp/util/arena.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

typedef ArenaImpl<32 * 1024> RequestArena;
typedef boost::aligned_storage<sizeof(RequestArena),sizeof(void*)>::type
        RequestArenaStorage;
extern __thread RequestArenaStorage s_requestArenaStorage;

typedef Arena VarEnvArena;
typedef boost::aligned_storage<sizeof(VarEnvArena),sizeof(void*)>::type
        VarEnvArenaStorage;
extern __thread VarEnvArenaStorage s_varEnvArenaStorage;

/*
 * Access to a request-lifetime arena allocator.
 */
inline RequestArena& request_arena() {
  void* vp = &s_requestArenaStorage;
  return *static_cast<RequestArena*>(vp);
}

/*
 * Access to a arena that has allocation lifetimes matching the
 * current VarEnv.
 */
inline VarEnvArena& varenv_arena() {
  void* vp = &s_varEnvArenaStorage;
  return *static_cast<VarEnvArena*>(vp);
}

//////////////////////////////////////////////////////////////////////

}

#endif
