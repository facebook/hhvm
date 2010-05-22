/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __EXT_IPC_H__
#define __EXT_IPC_H__

#include <runtime/base/base_includes.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// message queue

int64 f_ftok(CStrRef pathname, CStrRef proj);

Variant f_msg_get_queue(int64 key, int64 perms = 0666);
bool f_msg_send(CObjRef queue, int64 msgtype, CVarRef message,
                bool serialize = true, bool blocking = true,
                Variant errorcode = null);
bool f_msg_receive(CObjRef queue, int64 desiredmsgtype, Variant msgtype,
                   int64 maxsize, Variant message, bool unserialize = true,
                   int64 flags = 0, Variant errorcode = null);
bool f_msg_remove_queue(CObjRef queue);
bool f_msg_set_queue(CObjRef queue, CArrRef data);
Array f_msg_stat_queue(CObjRef queue);

///////////////////////////////////////////////////////////////////////////////
// semaphore

bool f_sem_acquire(CObjRef sem_identifier);
Variant f_sem_get(int64 key, int64 max_acquire = 1, int64 perm = 0666,
                  bool auto_release = true);
bool f_sem_release(CObjRef sem_identifier);
bool f_sem_remove(CObjRef sem_identifier);

///////////////////////////////////////////////////////////////////////////////
// shared memory

Variant f_shm_attach(int64 shm_key, int64 shm_size = 10000, int64 shm_flag = 0666);
bool f_shm_detach(int64 shm_identifier);
bool f_shm_remove(int64 shm_identifier);
Variant f_shm_get_var(int64 shm_identifier, int64 variable_key);
bool f_shm_put_var(int64 shm_identifier, int64 variable_key, CVarRef variable);
bool f_shm_remove_var(int64 shm_identifier, int64 variable_key);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_IPC_H__
