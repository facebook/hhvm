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

#ifndef incl_HPHP_EXT_IPC_H_
#define incl_HPHP_EXT_IPC_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// message queue

int64_t f_ftok(const String& pathname, const String& proj);

Variant f_msg_get_queue(int64_t key, int64_t perms = 0666);
bool f_msg_queue_exists(int64_t key);
bool f_msg_send(CResRef queue, int64_t msgtype, CVarRef message,
                bool serialize = true, bool blocking = true,
                VRefParam errorcode = uninit_null());
bool f_msg_receive(CResRef queue, int64_t desiredmsgtype, VRefParam msgtype,
                   int64_t maxsize, VRefParam message, bool unserialize = true,
                   int64_t flags = 0, VRefParam errorcode = uninit_null());
bool f_msg_remove_queue(CResRef queue);
bool f_msg_set_queue(CResRef queue, CArrRef data);
Array f_msg_stat_queue(CResRef queue);

///////////////////////////////////////////////////////////////////////////////
// semaphore

bool f_sem_acquire(CResRef sem_identifier);
Variant f_sem_get(int64_t key, int64_t max_acquire = 1, int64_t perm = 0666,
                  bool auto_release = true);
bool f_sem_release(CResRef sem_identifier);
bool f_sem_remove(CResRef sem_identifier);

///////////////////////////////////////////////////////////////////////////////
// shared memory

Variant f_shm_attach(int64_t shm_key, int64_t shm_size = 10000, int64_t shm_flag = 0666);
bool f_shm_detach(int64_t shm_identifier);
bool f_shm_remove(int64_t shm_identifier);
Variant f_shm_get_var(int64_t shm_identifier, int64_t variable_key);
bool f_shm_has_var(int64_t shm_identifier, int64_t variable_key);
bool f_shm_put_var(int64_t shm_identifier, int64_t variable_key, CVarRef variable);
bool f_shm_remove_var(int64_t shm_identifier, int64_t variable_key);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_IPC_H_
