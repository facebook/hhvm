/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ipc/ext_ipc.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/system/constants.h"
#include "hphp/util/lock.h"
#include "hphp/util/alloc.h"
#include <folly/String.h>

#include <memory>
#include <set>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#ifdef __CYGWIN__
/* These is missing from cygwin ipc headers. */
#define MSG_EXCEPT 02000
#endif

#if defined(__APPLE__) || defined(__CYGWIN__)
/* OS X defines msgbuf, but it is defined with extra fields and some weird
 * types. It turns out that the actual msgsnd() and msgrcv() calls work fine
 * with the same structure that other OSes use. This is weird, but this is also
 * what PHP does, so *shrug*. */
typedef struct {
    long mtype;
    char mtext[1];
} hhvm_msgbuf;
#else
typedef msgbuf hhvm_msgbuf;
#endif

using HPHP::ScopedMem;

namespace HPHP {

static class SysvmsgExtension final : public Extension {
public:
  SysvmsgExtension() : Extension("sysvmsg", NO_EXTENSION_VERSION_YET) {}
  void moduleInit() override {
    HHVM_FE(ftok);
    HHVM_FE(msg_get_queue);
    HHVM_FE(msg_queue_exists);
    HHVM_FE(msg_send);
    HHVM_FE(msg_receive);
    HHVM_FE(msg_remove_queue);
    HHVM_FE(msg_set_queue);
    HHVM_FE(msg_stat_queue);

    loadSystemlib();
  }
} s_sysvmsg_extension;

static class SysvsemExtension final : public Extension {
public:
  SysvsemExtension() : Extension("sysvsem", NO_EXTENSION_VERSION_YET) {}
  void moduleInit() override {
    HHVM_FE(sem_acquire);
    HHVM_FE(sem_get);
    HHVM_FE(sem_release);
    HHVM_FE(sem_remove);

    loadSystemlib();
  }
} s_sysvsem_extension;

static class SysvshmExtension final : public Extension {
public:
  SysvshmExtension() : Extension("sysvshm", NO_EXTENSION_VERSION_YET) {}
  void moduleInit() override {
    HHVM_FE(shm_attach);
    HHVM_FE(shm_detach);
    HHVM_FE(shm_remove);
    HHVM_FE(shm_get_var);
    HHVM_FE(shm_has_var);
    HHVM_FE(shm_put_var);
    HHVM_FE(shm_remove_var);

    loadSystemlib();
  }
} s_sysvshm_extension;

///////////////////////////////////////////////////////////////////////////////

int64_t HHVM_FUNCTION(ftok,
                      const String& pathname,
                      const String& proj) {
  if (pathname.empty()) {
    raise_warning("Pathname is empty");
    return -1;
  }
  if (proj.length() != 1) {
    raise_warning("Project identifier has to be one character int64: %s",
                  proj.c_str());
    return -1;
  }
  return ftok(pathname.c_str(), (int)proj[0]);
}

///////////////////////////////////////////////////////////////////////////////
// message queue

class MessageQueue : public ResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(MessageQueue)

  int64_t key;
  int id;

  CLASSNAME_IS("MessageQueue");
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }
};

Variant HHVM_FUNCTION(msg_get_queue,
                      int64_t key,
                      int64_t perms /* = 0666 */) {
  int id = msgget(key, 0);
  if (id < 0) {
    id = msgget(key, IPC_CREAT | IPC_EXCL | perms);
    if (id < 0) {
      raise_warning("Failed to create message queue for key 0x%" PRIx64 ": %s",
                      key, folly::errnoStr(errno).c_str());
      return false;
    }
  }
  auto q = req::make<MessageQueue>();
  q->key = key;
  q->id = id;
  return Variant(std::move(q));
}

bool HHVM_FUNCTION(msg_queue_exists,
                   int64_t key) {
  return msgget(key, 0) >= 0;
}

bool HHVM_FUNCTION(msg_remove_queue,
                   const Resource& queue) {
  auto q = cast<MessageQueue>(queue);
  if (!q) {
    raise_warning("Invalid message queue was specified");
    return false;
  }

  return msgctl(q->id, IPC_RMID, NULL) == 0;
}

const StaticString
  s_msg_perm_uid("msg_perm.uid"),
  s_msg_perm_gid("msg_perm.gid"),
  s_msg_perm_mode("msg_perm.mode"),
  s_msg_stime("msg_stime"),
  s_msg_rtime("msg_rtime"),
  s_msg_ctime("msg_ctime"),
  s_msg_qnum("msg_qnum"),
  s_msg_qbytes("msg_qbytes"),
  s_msg_lspid("msg_lspid"),
  s_msg_lrpid("msg_lrpid");

bool HHVM_FUNCTION(msg_set_queue,
                   const Resource& queue,
                   const Array& data) {
  auto q = cast<MessageQueue>(queue);
  if (!q) {
    raise_warning("Invalid message queue was specified");
    return false;
  }

  struct msqid_ds stat;
  if (msgctl(q->id, IPC_STAT, &stat) == 0) {
    Variant value;
    value = data[s_msg_perm_uid];
    if (!value.isNull()) stat.msg_perm.uid = value.toInt64();
    value = data[s_msg_perm_gid];
    if (!value.isNull()) stat.msg_perm.uid = value.toInt64();
    value = data[s_msg_perm_mode];
    if (!value.isNull()) stat.msg_perm.uid = value.toInt64();
    value = data[s_msg_qbytes];
    if (!value.isNull()) stat.msg_perm.uid = value.toInt64();

    return msgctl(q->id, IPC_SET, &stat) == 0;
  }

  return false;
}

Array HHVM_FUNCTION(msg_stat_queue,
                    const Resource& queue) {
  auto q = cast<MessageQueue>(queue);
  if (!q) {
    raise_warning("Invalid message queue was specified");
    return Array();
  }

  struct msqid_ds stat;
  if (msgctl(q->id, IPC_STAT, &stat) == 0) {
    Array data;
    data.set(s_msg_perm_uid,  (int64_t)stat.msg_perm.uid);
    data.set(s_msg_perm_gid,  (int64_t)stat.msg_perm.gid);
    data.set(s_msg_perm_mode, (int32_t)stat.msg_perm.mode);
    data.set(s_msg_stime,     (int64_t)stat.msg_stime);
    data.set(s_msg_rtime,     (int64_t)stat.msg_rtime);
    data.set(s_msg_ctime,     (int64_t)stat.msg_ctime);
    data.set(s_msg_qnum,      (int64_t)stat.msg_qnum);
    data.set(s_msg_qbytes,    (int64_t)stat.msg_qbytes);
    data.set(s_msg_lspid,     stat.msg_lspid);
    data.set(s_msg_lrpid,     stat.msg_lrpid);
    return data;
  }

  return Array();
}

bool HHVM_FUNCTION(msg_send,
                   const Resource& queue,
                   int64_t msgtype,
                   const Variant& message,
                   bool serialize /* = true */,
                   bool blocking /* = true */,
                   VRefParam errorcode /* = null */) {
  auto q = cast<MessageQueue>(queue);
  if (!q) {
    raise_warning("Invalid message queue was specified");
    return false;
  }

  hhvm_msgbuf *buffer = nullptr;
  String data;
  if (serialize) {
    data = HHVM_FN(serialize)(message);
  } else {
    data = message.toString();
  }
  int len = data.length();
  buffer = (hhvm_msgbuf *)calloc(len + sizeof(hhvm_msgbuf), 1);
  ScopedMem deleter(buffer);
  buffer->mtype = msgtype;
  memcpy(buffer->mtext, data.c_str(), len + 1);

  int result = msgsnd(q->id, buffer, len, blocking ? 0 : IPC_NOWAIT);
  if (result < 0) {
    int err = errno;
    raise_warning("Unable to send message: %s",
                    folly::errnoStr(err).c_str());
    errorcode = err;
    return false;
  }
  return true;
}

bool HHVM_FUNCTION(msg_receive,
                   const Resource& queue,
                   int64_t desiredmsgtype,
                   VRefParam msgtype,
                   int64_t maxsize, VRefParam message,
                   bool unserialize /* = true */,
                   int64_t flags /* = 0 */,
                   VRefParam errorcode /* = null */) {
  auto q = cast<MessageQueue>(queue);
  if (!q) {
    raise_warning("Invalid message queue was specified");
    return false;
  }

  if (maxsize <= 0) {
    raise_warning("Maximum size of the message has to be greater than zero");
    return false;
  }

  int64_t realflags = 0;
  if (flags != 0) {
#if !defined(__APPLE__) && !defined(__FreeBSD__)
    if (flags & k_MSG_EXCEPT) realflags |= MSG_EXCEPT;
#endif
    if (flags & k_MSG_NOERROR) realflags |= MSG_NOERROR;
    if (flags & k_MSG_IPC_NOWAIT) realflags |= IPC_NOWAIT;
  }

  hhvm_msgbuf *buffer = (hhvm_msgbuf *)calloc(maxsize + sizeof(hhvm_msgbuf), 1);
  ScopedMem deleter(buffer);

  int result = msgrcv(q->id, buffer, maxsize, desiredmsgtype, realflags);
  if (result < 0) {
    errorcode = errno;
    return false;
  }

  msgtype = (int)buffer->mtype;
  if (unserialize) {
    const char *bufText = (const char *)buffer->mtext;
    uint32_t bufLen = strlen(bufText);
    VariableUnserializer vu(bufText, bufLen,
                            VariableUnserializer::Type::Serialize);
    try {
      message = vu.unserialize();
    } catch (ResourceExceededException&) {
      throw;
    } catch (Exception&) {
      raise_warning("Message corrupted");
      return false;
    }
  } else {
    message = String((const char *)buffer->mtext);
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// semaphore

/* Semaphore functions using System V semaphores.  Each semaphore
 * actually consists of three semaphores allocated as a unit under the
 * same key.  Semaphore 0 (SYSVSEM_SEM) is the actual semaphore, it is
 * initialized to max_acquire and decremented as processes acquire it.
 * The value of semaphore 1 (SYSVSEM_USAGE) is a count of the number
 * of processes using the semaphore.  After calling semget(), if a
 * process finds that the usage count is 1, it will set the value of
 * SYSVSEM_SEM to max_acquire.  This allows max_acquire to be set and
 * track the PHP code without having a global init routine or external
 * semaphore init code.  Except see the bug regarding a race condition
 * php_sysvsem_get().  Semaphore 2 (SYSVSEM_SETVAL) serializes the
 * calls to GETVAL SYSVSEM_USAGE and SETVAL SYSVSEM_SEM.  It can be
 * acquired only when it is zero.
 */
#define SYSVSEM_SEM    0
#define SYSVSEM_USAGE  1
#define SYSVSEM_SETVAL 2

union semun {
  int val;                  /* value for SETVAL */
  struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
  unsigned short *array;    /* array for GETALL, SETALL */
  /* Linux specific part: */
  struct seminfo *__buf;    /* buffer for IPC_INFO */
};

struct Semaphore : SweepableResourceData {
  int key;          // For error reporting.
  int semid;        // Returned by semget()
  int count;        // Acquire count for auto-release.
  int auto_release; // flag that says to auto-release.

  CLASSNAME_IS("Semaphore");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  bool op(bool acquire) {
    struct sembuf sop;

    if (!acquire && count == 0) {
      raise_warning("SysV semaphore %d (key 0x%x) is not currently acquired",
                      o_getId(), key);
      return false;
    }

    sop.sem_num = SYSVSEM_SEM;
    sop.sem_op  = acquire ? -1 : 1;
    sop.sem_flg = SEM_UNDO;

    while (semop(semid, &sop, 1) == -1) {
      if (errno != EINTR) {
        raise_warning("failed to %s key 0x%x: %s",
                        acquire ? "acquire" : "release",
                        key, folly::errnoStr(errno).c_str());
        return false;
      }
    }

    count -= acquire ? -1 : 1;
    return true;
  }

  ~Semaphore() {
    /*
     * if count == -1, semaphore has been removed
     * Need better way to handle this
     */
    if (count == -1 || !auto_release) {
      return;
    }

    sembuf sop[2];
    int opcount = 1;

    /* Decrement the usage count. */
    sop[0].sem_num = SYSVSEM_USAGE;
    sop[0].sem_op  = -1;
    sop[0].sem_flg = SEM_UNDO;

    /* Release the semaphore if it has been acquired but not released. */
    if (count) {
      sop[1].sem_num = SYSVSEM_SEM;
      sop[1].sem_op  = count;
      sop[1].sem_flg = SEM_UNDO;
      opcount = 2;
    }

    semop(semid, sop, opcount);
  }
  DECLARE_RESOURCE_ALLOCATION(Semaphore);
};

IMPLEMENT_RESOURCE_ALLOCATION(Semaphore)

bool HHVM_FUNCTION(sem_acquire,
                   const Resource& sem_identifier) {
  return cast<Semaphore>(sem_identifier)->op(true);
}

bool HHVM_FUNCTION(sem_release,
                   const Resource& sem_identifier) {
  return cast<Semaphore>(sem_identifier)->op(false);
}

/**
 * Return an id for the semaphore with the given key, and allow max_acquire
 * (default 1) processes to acquire it simultaneously.
 */
Variant HHVM_FUNCTION(sem_get,
                      int64_t key,
                      int64_t max_acquire /* = 1 */,
                      int64_t perm /* = 0666 */,
                      bool auto_release /* = true */) {
  /* Get/create the semaphore.  Note that we rely on the semaphores
   * being zeroed when they are created.  Despite the fact that
   * the(?)  Linux semget() man page says they are not initialized,
   * the kernel versions 2.0.x and 2.1.z do in fact zero them.
   */
  int semid = semget(key, 3, perm|IPC_CREAT);
  if (semid == -1) {
    raise_warning("failed for key 0x%" PRIx64 ": %s", key,
                    folly::errnoStr(errno).c_str());
    return false;
  }

  /* Find out how many processes are using this semaphore.  Note
   * that on Linux (at least) there is a race condition here because
   * semaphore undo on process exit is not atomic, so we could
   * acquire SYSVSEM_SETVAL before a crashed process has decremented
   * SYSVSEM_USAGE in which case count will be greater than it
   * should be and we won't set max_acquire.  Fortunately this
   * doesn't actually matter in practice.
   */

  /* Wait for sem 1 to be zero . . . */
  struct sembuf sop[3];
  sop[0].sem_num = SYSVSEM_SETVAL;
  sop[0].sem_op  = 0;
  sop[0].sem_flg = 0;

  /* . . . and increment it so it becomes non-zero . . . */
  sop[1].sem_num = SYSVSEM_SETVAL;
  sop[1].sem_op  = 1;
  sop[1].sem_flg = SEM_UNDO;

  /* . . . and increment the usage count. */
  sop[2].sem_num = SYSVSEM_USAGE;
  sop[2].sem_op  = 1;
  sop[2].sem_flg = SEM_UNDO;

  while (semop(semid, sop, 3) == -1) {
    if (errno != EINTR) {
      raise_warning("failed acquiring SYSVSEM_SETVAL for key 0x%" PRIx64 ": %s",
                      key, folly::errnoStr(errno).c_str());
      break;
    }
  }

  /* Get the usage count. */
  int count = semctl(semid, SYSVSEM_USAGE, GETVAL, NULL);
  if (count == -1) {
    raise_warning("failed for key 0x%" PRIx64 ": %s", key,
                    folly::errnoStr(errno).c_str());
  }

  /* If we are the only user, then take this opportunity to set the max. */
  if (count == 1) {
    union semun semarg;
    semarg.val = max_acquire;
    if (semctl(semid, SYSVSEM_SEM, SETVAL, semarg) == -1) {
      raise_warning("failed for key 0x%" PRIx64 ": %s", key,
                      folly::errnoStr(errno).c_str());
    }
  }

  /* Set semaphore 1 back to zero. */
  sop[0].sem_num = SYSVSEM_SETVAL;
  sop[0].sem_op  = -1;
  sop[0].sem_flg = SEM_UNDO;
  while (semop(semid, sop, 1) == -1) {
    if (errno != EINTR) {
      raise_warning("failed releasing SYSVSEM_SETVAL for key 0x%" PRIx64 ": %s",
                      key, folly::errnoStr(errno).c_str());
      break;
    }
  }

  auto sem_ptr = req::make<Semaphore>();
  sem_ptr->key   = key;
  sem_ptr->semid = semid;
  sem_ptr->count = 0;
  sem_ptr->auto_release = auto_release;
  return Resource(sem_ptr);
}

/**
 * contributed by Gavin Sherry gavin@linuxworld.com.au
 * Fri Mar 16 00:50:13 EST 2001
 */
bool HHVM_FUNCTION(sem_remove,
                   const Resource& sem_identifier) {
  auto sem_ptr = cast<Semaphore>(sem_identifier);

  union semun un;
  struct semid_ds buf;
  un.buf = &buf;
  if (semctl(sem_ptr->semid, 0, IPC_STAT, un) < 0) {
    raise_warning("SysV semaphore %d does not (any longer) exist",
                    sem_identifier->o_getId());
    return false;
  }

  if (semctl(sem_ptr->semid, 0, IPC_RMID, un) < 0) {
    raise_warning("failed for SysV sempphore %d: %s",
                    sem_identifier->o_getId(),
                    folly::errnoStr(errno).c_str());
    return false;
  }

  /* let release_sysvsem_sem know we have removed
   * the semaphore to avoid issues with releasing.
   */
  sem_ptr->count = -1;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// shared memory

typedef struct {
  long key;
  long length;
  long next;
  char mem;
} sysvshm_chunk;

typedef struct {
  char magic[8];
  long start;
  long end;
  long free;
  long total;
} sysvshm_chunk_head;

class sysvshm_shm {
public:
  key_t key;               /* Key set by user */
  long id;                 /* Returned by shmget. */
  sysvshm_chunk_head *ptr; /* memoryaddress of shared memory */

  ~sysvshm_shm() {
    shmdt((void *)ptr);
  }
};

class shm_set : public std::set<sysvshm_shm*> {
public:
  ~shm_set() {
    for (auto iter = begin(); iter != end(); ++iter) {
      delete *iter;
    }
  }
};

static Mutex g_shm_mutex;
static shm_set g_shms;

static long check_shm_data(sysvshm_chunk_head *ptr, long key) {
  long pos;
  sysvshm_chunk *shm_var;

  pos = ptr->start;

  for (;;) {
    if (pos >= ptr->end) {
      return -1;
    }
    shm_var = (sysvshm_chunk*) ((char *) ptr + pos);
    if (shm_var->key == key) {
      return pos;
    }
    pos += shm_var->next;

    if (shm_var->next <= 0 || pos < ptr->start) {
      return -1;
    }
  }
  return -1;
}

static int remove_shm_data(sysvshm_chunk_head *ptr, long shm_varpos) {
  sysvshm_chunk *chunk_ptr, *next_chunk_ptr;
  long memcpy_len;

  chunk_ptr = (sysvshm_chunk *) ((char *) ptr + shm_varpos);
  next_chunk_ptr =
    (sysvshm_chunk *) ((char *) ptr + shm_varpos + chunk_ptr->next);

  memcpy_len = ptr->end-shm_varpos - chunk_ptr->next;
  ptr->free += chunk_ptr->next;
  ptr->end -= chunk_ptr->next;
  if (memcpy_len > 0) {
    memcpy(chunk_ptr, next_chunk_ptr, memcpy_len);
  }
  return 0;
}

static int put_shm_data(sysvshm_chunk_head *ptr, long key, char *data,
                        long len) {
  sysvshm_chunk *shm_var;
  long total_size;
  long shm_varpos;

  total_size = ((long) (len + sizeof(sysvshm_chunk) - 1) / 4) * 4 +
    4; /* 4-byte alligment */

  if ((shm_varpos = check_shm_data(ptr, key)) > 0) {
    remove_shm_data(ptr, shm_varpos);
  }

  if (ptr->free < total_size) {
    return -1; /* not enough memeory */
  }

  shm_var = (sysvshm_chunk *) ((char *) ptr + ptr->end);
  shm_var->key = key;
  shm_var->length = len;
  shm_var->next = total_size;
  memcpy(&(shm_var->mem), data, len);
  ptr->end += total_size;
  ptr->free -= total_size;
  return 0;
}

Variant HHVM_FUNCTION(shm_attach,
                      int64_t shm_key,
                      int64_t shm_size /* = 10000 */,
                      int64_t shm_flag /* = 0666 */) {
  char *shm_ptr;
  long shm_id;

  if (shm_size < 1) {
    raise_warning("Segment size must be greater than zero.");
    return false;
  }

  std::unique_ptr<sysvshm_shm> shm_list_ptr(new sysvshm_shm());

  /* get the id from a specified key or create new shared memory */
  if ((shm_id = shmget(shm_key, 0, 0)) < 0) {
    if (shm_size < (int)sizeof(sysvshm_chunk_head)) {
      raise_warning("failed for key 0x%" PRIx64 ": memorysize too small", shm_key);
      return false;
    }
    if ((shm_id = shmget(shm_key, shm_size, shm_flag | IPC_CREAT | IPC_EXCL))
        < 0) {
      raise_warning("failed for key 0x%" PRIx64 ": %s", shm_key,
                      folly::errnoStr(errno).c_str());
      return false;
    }
  }

  if ((shm_ptr = (char*)shmat(shm_id, NULL, 0)) == (char *)-1) {
    raise_warning("failed for key 0x%" PRIx64 ": %s", shm_key,
                    folly::errnoStr(errno).c_str());
    return false;
  }

  /* check if shm is already initialized */
  sysvshm_chunk_head *chunk_ptr = (sysvshm_chunk_head *)shm_ptr;
  if (strcmp((char*) &(chunk_ptr->magic), "PHP_SM") != 0) {
    strcpy((char*) &(chunk_ptr->magic), "PHP_SM");
    chunk_ptr->start = sizeof(sysvshm_chunk_head);
    chunk_ptr->end = chunk_ptr->start;
    chunk_ptr->total = shm_size;
    chunk_ptr->free = shm_size-chunk_ptr->end;
  }

  shm_list_ptr->key = shm_key;
  shm_list_ptr->id = shm_id;
  shm_list_ptr->ptr = chunk_ptr;
  Lock lock(g_shm_mutex);
  int64_t ret = (int64_t)shm_list_ptr.get();
  g_shms.insert(shm_list_ptr.release());
  return ret;
}

bool HHVM_FUNCTION(shm_detach,
                   int64_t shm_identifier) {
  Lock lock(g_shm_mutex);
  auto iter = g_shms.find((sysvshm_shm*)shm_identifier);
  if (iter == g_shms.end()) {
    raise_warning("%" PRId64 " is not a SysV shared memory index",
                  shm_identifier);
    return false;
  }
  g_shms.erase(iter);
  delete (sysvshm_shm*)shm_identifier;
  return true;
}

bool HHVM_FUNCTION(shm_remove,
                   int64_t shm_identifier) {
  Lock lock(g_shm_mutex);
  auto iter = g_shms.find((sysvshm_shm*)shm_identifier);
  if (iter == g_shms.end()) {
    raise_warning("%" PRId64 " is not a SysV shared memory index", shm_identifier);
    return false;
  }
  sysvshm_shm *shm_list_ptr = *iter;

  if (shmctl(shm_list_ptr->id, IPC_RMID,NULL) < 0) {
    raise_warning(
#ifdef __CYGWIN__
      // key is a long long int in cygwin
      "failed for key 0x%lld, id %" PRId64 ": %s",
#else
      "failed for key 0x%x, id %" PRId64 ": %s",
#endif
      shm_list_ptr->key, shm_identifier, folly::errnoStr(errno).c_str()
    );
    return false;
  }
  return true;
}

Variant HHVM_FUNCTION(shm_get_var,
                      int64_t shm_identifier,
                      int64_t variable_key) {
  Lock lock(g_shm_mutex);
  auto iter = g_shms.find((sysvshm_shm*)shm_identifier);
  if (iter == g_shms.end()) {
    raise_warning("%" PRId64 " is not a SysV shared memory index",
                  shm_identifier);
    return false;
  }
  sysvshm_shm *shm_list_ptr = *iter;

  long shm_varpos = check_shm_data(shm_list_ptr->ptr, variable_key);
  if (shm_varpos < 0) {
    return false;
  }

  sysvshm_chunk *shm_var =
    (sysvshm_chunk*)((char *)shm_list_ptr->ptr + shm_varpos);
  return unserialize_from_buffer(&shm_var->mem, shm_var->length);
}

bool HHVM_FUNCTION(shm_has_var,
                   int64_t shm_identifier,
                   int64_t variable_key) {
  Lock lock(g_shm_mutex);
  auto iter = g_shms.find((sysvshm_shm*)shm_identifier);
  if (iter == g_shms.end()) {
    raise_warning("%" PRId64 " is not a SysV shared memory index",
                  shm_identifier);
    return false;
  }
  sysvshm_shm *shm_list_ptr = *iter;

  long shm_varpos = check_shm_data(shm_list_ptr->ptr, variable_key);
  return shm_varpos >= 0;
}

bool HHVM_FUNCTION(shm_put_var,
                   int64_t shm_identifier,
                   int64_t variable_key,
                   const Variant& variable) {
  /* setup string-variable and serialize */
  String serialized = HHVM_FN(serialize)(variable);

  Lock lock(g_shm_mutex);
  auto iter = g_shms.find((sysvshm_shm*)shm_identifier);
  if (iter == g_shms.end()) {
    raise_warning("%" PRId64 " is not a SysV shared memory index",
                  shm_identifier);
    return false;
  }

  sysvshm_shm *shm_list_ptr = *iter;
  /* insert serialized variable into shared memory */
  int ret = put_shm_data(shm_list_ptr->ptr, variable_key,
                         (char*)serialized.data(), serialized.size());
  if (ret == -1) {
    raise_warning("not enough shared memory left");
    return false;
  }
  return true;
}

bool HHVM_FUNCTION(shm_remove_var,
                   int64_t shm_identifier,
                   int64_t variable_key) {
  Lock lock(g_shm_mutex);
  auto iter = g_shms.find((sysvshm_shm*)shm_identifier);
  if (iter == g_shms.end()) {
    raise_warning("%" PRId64 " is not a SysV shared memory index", shm_identifier);
    return false;
  }
  sysvshm_shm *shm_list_ptr = *iter;

  long shm_varpos = check_shm_data(shm_list_ptr->ptr, variable_key);
  if (shm_varpos < 0) {
    raise_warning("variable key %" PRId64 " doesn't exist", variable_key);
    return false;
  }
  remove_shm_data(shm_list_ptr->ptr, shm_varpos);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
