/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/util/rds-local.h"

#include <sys/shm.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

// ShmRec is the per-shared memory data.  Each shmop_open() creates one of these
// and it lives until the shmid is closed.
struct ShmRec {
  ~ShmRec() {
    if (m_addr) {
      shmdt(m_addr);
      m_addr = nullptr;
    }
  }

  ShmRec() : m_shmid(0), m_readonly(false), m_size(0), m_addr(nullptr) { }

  bool _delete() {
    if (shmctl(m_shmid, IPC_RMID, nullptr) == -1) {
      raise_warning(
        "shmop_delete(): can't mark segment for deletion (are you the owner?)");
      return false;
    }

    return true;
  }

  bool open(int64_t key, const StringData* flags, int64_t mode, int64_t size) {

    if (flags->size() != 1) {
      raise_warning("shmop_open(): %s is not a valid flag",
                    flags->toCppString().c_str());
    }

    int shmflg = 0;
    int shmatflg = 0;

    switch (flags->data()[0]) {
      case 'a': // access
        m_readonly = true;
        shmatflg = SHM_RDONLY;
        break;

      case 'c': // create
        shmflg = IPC_CREAT;
        m_size = size;
        break;

      case 'w': // read & write
        break;

      case 'n': // new
        shmflg = IPC_CREAT | IPC_EXCL;
        m_size = size;
        break;

      default:
        raise_warning("shmop_open(): invalid access mode");
        return false;
    }

    if (shmflg & IPC_CREAT && m_size < 1) {
      raise_warning(
        "shmop_open(): Shared memory segment size must be greater than zero");
      return false;
    }

    // NOTE: Deviation from zend's implementation.  Zend just ignores the high
    // bits of the key but that's a potential problem.
    if ((key_t)key != key) {
      raise_warning(
        "shmop_open(): invalid shared memory key (key must be 32-bit)");
      return false;
    }

    m_shmid = shmget(key, m_size, shmflg | (mode & 0777));
    if (m_shmid == -1) {
      raise_warning(
        "shmop_open(): unable to attach or create shared memory segment");
      return false;
    }

    shmid_ds ds;
    if (shmctl(m_shmid, IPC_STAT, &ds) == -1) {
      raise_warning(
        "shmop_open(): unable to get shared memory segment information");
      return false;
    }
    m_size = ds.shm_segsz;

    void* addr = shmat(m_shmid, nullptr, shmatflg);
    if (reinterpret_cast<ptrdiff_t>(addr) == -1) {
      raise_warning("shmop_open(): unable to attach to shared memory segment");
      return false;
    }
    m_addr = reinterpret_cast<char*>(addr);

    return true;
  }

  Variant read(int64_t start, int64_t count) {
    if (start < 0 || start > m_size) {
      raise_warning("shmop_read(): start is out of range");
      return Variant(false);
    }

    if (count < 0 || (m_size - start - count) < 0) {
      raise_warning("shmop_read(): count is out of range");
      return Variant(false);
    }

    return Variant(String(&m_addr[start], count, CopyString));
  }

  int64_t size() {
    return m_size;
  }

  Variant write(const String& data, int64_t offset) {
    if (m_readonly) {
      raise_warning("shmop_write(): trying to write to a read only segment");
      return Variant(false);
    }

    if (offset < 0 || offset > m_size) {
      raise_warning("shmop_write(): offset out of range");
      return Variant(false);
    }

    size_t len = std::min<size_t>(data.size(), m_size - offset);
    memcpy(&m_addr[offset], data.data(), len);

    return Variant(len);
  }

private:
  int m_shmid;
  bool m_readonly;
  int64_t m_size;
  char* m_addr;
};

struct ShmopRequestLocal final : RequestEventHandler {
  void requestInit() override {
    // no-op
  }

  void requestShutdown() override {
    m_records.clear();
  }

  ShmRec* findShm(const char* functionName, int64_t shmid) {
    auto const it = m_records.find(shmid);
    if (it == m_records.end()) {
      raise_warning("%s(): no shared memory segment with an id of"
                    " [%" PRId64 "]", functionName, shmid);
      return nullptr;
    } else {
      return it->second.get();
    }
  }

  int64_t insertShm(std::unique_ptr<ShmRec> p) {
    // Use our ShmRec address as the shmid.  We'll check it to make sure it's
    // still valid when we use it but this way it's unique per process (although
    // it may get reused).
    int64_t shmid = reinterpret_cast<ssize_t>(p.get());
    assertx(LIKELY(m_records.find(shmid) == m_records.end()));
    m_records.emplace(shmid, std::move(p));
    return shmid;
  }

  void deleteRecord(int64_t shmid) {
    m_records.erase(m_records.find(shmid));
  }

  // Mapping from shmid -> ShmRec.  Since shmid is just &ShmRec we could do some
  // pointer trickery to use an unordered_set<> instead but... yuck.
  std::unordered_map<int64_t, std::unique_ptr<ShmRec> > m_records;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(ShmopRequestLocal, s_data);

void HHVM_FUNCTION(shmop_close, int64_t shmid) {
  auto shm = s_data->findShm("shmop_close", shmid);
  if (!shm) return;
  s_data->deleteRecord(shmid);
}

bool HHVM_FUNCTION(shmop_delete, int64_t shmid) {
  auto shm = s_data->findShm("shmop_delete", shmid);
  if (!shm) return false;
  return shm->_delete();
}

Variant HHVM_FUNCTION(shmop_open,
                             int64_t key,
                             const String& flags,
                             int64_t mode,
                             int64_t size) {

  auto shminfo = std::make_unique<ShmRec>();
  if (!shminfo->open(key, flags.get(), mode, size)) {
    return Variant(false);
  }

  return Variant(s_data->insertShm(std::move(shminfo)));
}

Variant HHVM_FUNCTION(shmop_read,
                             int64_t shmid,
                             int64_t start,
                             int64_t count) {
  auto shm = s_data->findShm("shmop_read", shmid);
  if (!shm) return false;
  return shm->read(start, count);
}

int64_t HHVM_FUNCTION(shmop_size, int64_t shmid) {
  auto shm = s_data->findShm("shmop_size", shmid);
  if (!shm) return false;
  return shm->size();
}

Variant HHVM_FUNCTION(shmop_write,
                             int64_t shmid,
                             const String& data,
                             int64_t offset) {
  auto shm = s_data->findShm("shmop_write", shmid);
  if (!shm) return false;
  return shm->write(data, offset);
}

struct ShmopExtension final : Extension {
  ShmopExtension() : Extension("shmop", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) { }

  void moduleRegisterNative() override {
    HHVM_FE(shmop_close);
    HHVM_FE(shmop_delete);
    HHVM_FE(shmop_open);
    HHVM_FE(shmop_read);
    HHVM_FE(shmop_size);
    HHVM_FE(shmop_write);
  }

} s_shmop_extension;

///////////////////////////////////////////////////////////////////////////////
}
