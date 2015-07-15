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
#include "hphp/runtime/base/user-file.h"

#include <cstdio>
#include <cassert>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/ext/stream/ext_stream.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

#define PHP_LOCK_SH 1
#define PHP_LOCK_EX 2
#define PHP_LOCK_UN 3
#define PHP_LOCK_NB 4

/* coerce the stream into some other form */
  /* cast as a stdio FILE * */
#define PHP_STREAM_AS_STDIO         0
  /* cast as a POSIX fd or socketd */
#define PHP_STREAM_AS_FD            1
  /* cast as a socketd */
#define PHP_STREAM_AS_SOCKETD       2
  /* cast as fd/socket for select purposes */
#define PHP_STREAM_AS_FD_FOR_SELECT 3

StaticString s_stream_open("stream_open");
StaticString s_stream_close("stream_close");
StaticString s_stream_read("stream_read");
StaticString s_stream_write("stream_write");
StaticString s_stream_seek("stream_seek");
StaticString s_stream_tell("stream_tell");
StaticString s_stream_eof("stream_eof");
StaticString s_stream_flush("stream_flush");
StaticString s_stream_truncate("stream_truncate");
StaticString s_stream_lock("stream_lock");
StaticString s_stream_stat("stream_stat");
StaticString s_stream_metadata("stream_metadata");
StaticString s_stream_cast("stream_cast");
StaticString s_url_stat("url_stat");
StaticString s_unlink("unlink");
StaticString s_rename("rename");
StaticString s_mkdir("mkdir");
StaticString s_rmdir("rmdir");

///////////////////////////////////////////////////////////////////////////////

UserFile::UserFile(Class *cls,
                   const req::ptr<StreamContext>& context /*= null */)
: UserFSNode(cls, context) {
  m_StreamOpen  = lookupMethod(s_stream_open.get());
  m_StreamClose = lookupMethod(s_stream_close.get());
  m_StreamRead  = lookupMethod(s_stream_read.get());
  m_StreamWrite = lookupMethod(s_stream_write.get());
  m_StreamSeek  = lookupMethod(s_stream_seek.get());
  m_StreamTell  = lookupMethod(s_stream_tell.get());
  m_StreamEof   = lookupMethod(s_stream_eof.get());
  m_StreamFlush = lookupMethod(s_stream_flush.get());
  m_StreamTruncate = lookupMethod(s_stream_truncate.get());
  m_StreamLock  = lookupMethod(s_stream_lock.get());
  m_StreamStat  = lookupMethod(s_stream_stat.get());
  m_UrlStat     = lookupMethod(s_url_stat.get());
  m_Unlink      = lookupMethod(s_unlink.get());
  m_Rename      = lookupMethod(s_rename.get());
  m_Mkdir       = lookupMethod(s_mkdir.get());
  m_Rmdir       = lookupMethod(s_rmdir.get());
  m_StreamMetadata = lookupMethod(s_stream_metadata.get());
  m_StreamCast  = lookupMethod(s_stream_cast.get());

  setIsLocal(true);

  // UserFile, to match Zend, should not call stream_close() unless it was ever
  // opened. This is a bit of a misuse of this field but the API doesn't allow
  // one direct access to an not-yet-opened stream resource so it should be
  // safe.
  setIsClosed(true);
}

UserFile::~UserFile() {
  if (!isClosed()) {
    close();
  }
}

void UserFile::sweep() {
  File::sweep();
}

///////////////////////////////////////////////////////////////////////////////

Resource UserFile::invokeCast(int castas) {
  bool invoked = false;
  Variant ret = invoke(
    m_StreamCast,
    s_stream_cast,
    PackedArrayInit(1)
      .append(castas)
      .toArray(),
    invoked
  );

  if (!invoked) {
    raise_warning(
      "%s::stream_cast is not implemented!",
      m_cls->name()->data()
    );
    return Resource();
  }
  if (ret.toBoolean() == false) {
    return Resource();
  }
  auto f = dyn_cast_or_null<File>(ret);
  if (!f) {
    raise_warning(
      "%s::stream_cast must return a stream resource",
      m_cls->name()->data()
    );
    return Resource();
  }
  if (f == this) {
    raise_warning(
      "%s::stream_cast must not return itself",
      m_cls->name()->data()
    );
    return Resource();
  }

  return Resource(std::move(f));
}

int UserFile::fd() const {
  Resource handle = const_cast<UserFile*>(this)->invokeCast(
    PHP_STREAM_AS_FD_FOR_SELECT);
  if (handle.isNull()) {
    return -1;
  }
  return cast<File>(handle)->fd();
}

///////////////////////////////////////////////////////////////////////////////

bool UserFile::openImpl(const String& filename, const String& mode,
                        int options) {
  // bool stream_open($path, $mode, $options, &$opened_path)
  bool invoked = false;
  Variant opened_path;
  Variant ret = invoke(
    m_StreamOpen,
    s_stream_open,
    PackedArrayInit(4)
      .append(filename)
      .append(mode)
      .append(options)
      .appendRef(opened_path)
      .toArray(),
    invoked
  );
  if (invoked && (ret.toBoolean() == true)) {
    setIsClosed(false);
    return true;
  }

  raise_warning("\"%s::stream_open\" call failed", m_cls->name()->data());
  return false;
}

bool UserFile::close() {
  // fclose() should prevent this from being called on a closed stream
  assert(!isClosed());

  // PHP's streams layer explicitly flushes on close
  // Mimick that for user-wrappers by pushing the flush here
  // without impacting other HPHP stream types.
  bool ret = flushImpl(false) || !RuntimeOption::CheckFlushOnUserClose;

  // void stream_close()
  invoke(m_StreamClose, s_stream_close, Array::Create());
  setIsClosed(true);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int64_t UserFile::readImpl(char *buffer, int64_t length) {
  // String stread_read($count)
  bool invoked = false;
  String str = invoke(m_StreamRead, s_stream_read,
                      make_packed_array(length), invoked);
  if (!invoked) {
    raise_warning("%s::stream_read is not implemented",
                  m_cls->name()->data());
    return 0;
  }

  int64_t didRead = str.size();
  if (didRead > length) {
    raise_warning("%s::stream_read - read %ld bytes more data than requested "
                  "(%ld read, %ld max) - excess data will be lost",
                  m_cls->name()->data(), (long)(didRead - length),
                  (long)didRead, (long)length);
    didRead = length;
  }
  memcpy(buffer, str.data(), didRead);
  return didRead;
}

int64_t UserFile::writeImpl(const char *buffer, int64_t length) {
  int64_t orig_length = length;
  // stream_write($data)
  while (length > 0) {
    bool invoked = false;
    int64_t didWrite = invoke(
      m_StreamWrite,
      s_stream_write,
      make_packed_array(String(buffer, length, CopyString)),
      invoked
    ).toInt64();
    if (!invoked) {
      raise_warning("%s::stream_write is not implemented",
                    m_cls->name()->data());
      return 0;
    }

    if (didWrite > length) {
      raise_warning("%s::stream_write - wrote %ld bytes more data than "
                    "requested (%ld written, %ld max)",
                    m_cls->name()->data(), (long)(didWrite - length),
                    (long)didWrite, (long)length);
      didWrite = length;
    } else if (didWrite <= 0) {
      break;
    }
    buffer += didWrite;
    length -= didWrite;
  }

  return orig_length - length;
}

///////////////////////////////////////////////////////////////////////////////

bool UserFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  assert(seekable());

  // Seek within m_buffer if we can, otherwise kill it and call user stream_seek / stream_tell
  if (whence == SEEK_CUR &&
      0 <= getReadPosition() + offset &&
           getReadPosition() + offset < getWritePosition()) {
    setReadPosition(getReadPosition() + offset);
    setPosition(getPosition() + offset);
    return true;
  } else if (whence == SEEK_SET &&
             0 <= offset - getPosition() + getReadPosition() &&
             offset - getPosition() + getReadPosition() < getWritePosition()) {
    setReadPosition(offset - getPosition() + getReadPosition());
    setPosition(offset);
    return true;
  } else {
    if (whence == SEEK_CUR) {
      offset += getReadPosition() - getWritePosition();
    }
    setReadPosition(0);
    setWritePosition(0);
  }

  // bool stream_seek($offset, $whence)
  bool invoked = false;
  bool sought  = invoke(
    m_StreamSeek, s_stream_seek, make_packed_array(offset, whence), invoked
  ).toBoolean();
  if (!invoked) {
    always_assert("No seek method? But I found one earlier?");
  }
  if (!sought) {
    return false;
  }

  // int stream_tell()
  Variant ret = invoke(m_StreamTell, s_stream_tell, Array::Create(), invoked);
  if (!invoked) {
    raise_warning("%s::stream_tell is not implemented!", m_cls->name()->data());
    return false;
  }
  setPosition(ret.isInteger() ? ret.toInt64() : -1);
  return true;
}

int64_t UserFile::tell() {
  return getPosition();
}

bool UserFile::eof() {
  // If there's data in the read buffer, then we're clearly not EOF
  if (bufferedLen() > 0) {
    return false;
  }

  // bool stream_eof()
  bool invoked = false;
  Variant ret = invoke(m_StreamEof, s_stream_eof, Array::Create(), invoked);
  if (!invoked) {
    return false;
  }
  return ret.isBoolean() ? ret.toBoolean() : true;
}

bool UserFile::flushImpl(bool strict) {
  // bool stream_flush()
  bool invoked = false;
  Variant ret = invoke(m_StreamFlush, s_stream_flush,
                       Array::Create(), invoked);
  if (!invoked) {
    return !strict;
  }
  return strict ? same(ret, true) : !same(ret, false);
}

bool UserFile::flush() {
  return flushImpl(true);
}

bool UserFile::truncate(int64_t size) {
  // bool stream_truncate()
  bool invoked = false;
  Variant ret = invoke(m_StreamTruncate, s_stream_truncate,
                       make_packed_array(size), invoked);
  if (!invoked) {
    return false;
  }
  return ret.isBoolean() ? ret.toBoolean() : false;
}

bool UserFile::lock(int operation, bool &wouldBlock) {
  int64_t op = 0;
  if (operation & LOCK_NB) {
    op |= PHP_LOCK_NB;
  }
  switch (operation & ~LOCK_NB) {
    case LOCK_SH: op |= PHP_LOCK_SH; break;
    case LOCK_EX: op |= PHP_LOCK_EX; break;
    case LOCK_UN: op |= PHP_LOCK_UN; break;
  }

  // bool stream_lock(int $operation)
  bool invoked = false;
  Variant ret = invoke(m_StreamLock, s_stream_lock,
                       make_packed_array(op), invoked);
  if (!invoked) {
    if (operation) {
      raise_warning("%s::stream_lock is not implemented!",
                    m_cls->name()->data());
    }
    return false;
  }
  return ret.isBoolean() ? ret.toBoolean() : false;
}

const StaticString
  s_dev("dev"),
  s_ino("ino"),
  s_mode("mode"),
  s_nlink("nlink"),
  s_uid("uid"),
  s_gid("gid"),
  s_rdev("rdev"),
  s_size("size"),
  s_atime("atime"),
  s_mtime("mtime"),
  s_ctime("ctime"),
  s_blksize("blksize"),
  s_blocks("blocks");

static int statFill(Variant stat_array, struct stat* stat_sb)
{
  if (!stat_array.isArray()) {
    return -1;
  }
  auto a = stat_array.getArrayData();
  stat_sb->st_dev = a->get(s_dev.get()).toInt64();
  stat_sb->st_ino = a->get(s_ino.get()).toInt64();
  stat_sb->st_mode = a->get(s_mode.get()).toInt64();
  stat_sb->st_nlink = a->get(s_nlink.get()).toInt64();
  stat_sb->st_uid = a->get(s_uid.get()).toInt64();
  stat_sb->st_gid = a->get(s_gid.get()).toInt64();
  stat_sb->st_rdev = a->get(s_rdev.get()).toInt64();
  stat_sb->st_size = a->get(s_size.get()).toInt64();
  stat_sb->st_atime = a->get(s_atime.get()).toInt64();
  stat_sb->st_mtime = a->get(s_mtime.get()).toInt64();
  stat_sb->st_ctime = a->get(s_ctime.get()).toInt64();
  stat_sb->st_blksize = a->get(s_blksize.get()).toInt64();
  stat_sb->st_blocks = a->get(s_blocks.get()).toInt64();
  return 0;
}

bool UserFile::stat(struct stat* stat_sb) {
  bool invoked = false;
  // array stream_stat ( )
  return statFill(invoke(m_StreamStat, s_stream_stat,
                         Array::Create(), invoked),
                  stat_sb) == 0;
}

int UserFile::urlStat(const String& path, struct stat* stat_sb,
                       int flags /* = 0 */) {
  // array url_stat ( string $path , int $flags )
  bool invoked = false;
  return statFill(invoke(m_UrlStat, s_url_stat,
                         make_packed_array(path, flags), invoked),
                  stat_sb);
}

static inline int simulateAccessResult(bool allowed) {
  if (allowed) {
    return 0;
  } else {
    errno = EACCES;
    return -1;
  }
}

int UserFile::access(const String& path, int mode) {
  struct stat buf;
  auto ret = urlStat(path, &buf, k_STREAM_URL_STAT_QUIET);
  if (ret < 0 || mode == F_OK) {
    return ret;
  }

  // The mode flags in stat are different from the flags used by access.
  auto uid = geteuid();
  auto gid = getegid();
  switch (mode) {
    case R_OK: // Test for read permission.
      return simulateAccessResult(
              (buf.st_uid == uid && (buf.st_mode & S_IRUSR)) ||
              (buf.st_gid == gid && (buf.st_mode & S_IRGRP)) ||
              (buf.st_mode & S_IROTH));
    case W_OK: // Test for write permission.
      return simulateAccessResult(
               (buf.st_uid == uid && (buf.st_mode & S_IWUSR)) ||
               (buf.st_gid == gid && (buf.st_mode & S_IWGRP)) ||
               (buf.st_mode & S_IWOTH));
    case X_OK: // Test for execute permission.
      return simulateAccessResult(
               !(buf.st_mode & S_IFDIR) &&  // Directories are not executable
               ((buf.st_uid == uid && (buf.st_mode & S_IXUSR)) ||
               (buf.st_gid == gid && (buf.st_mode & S_IXGRP)) ||
               (buf.st_mode & S_IXOTH)));
    default: // Unknown mode.
      return -1;
  }
}

int UserFile::lstat(const String& path, struct stat* buf) {
  return urlStat(path, buf, k_STREAM_URL_STAT_LINK);
}

int UserFile::stat(const String& path, struct stat* buf) {
  return urlStat(path, buf, k_STREAM_URL_STAT_QUIET);
}

bool UserFile::unlink(const String& filename) {
  // bool unlink($path)
  bool invoked = false;
  Variant ret = invoke(
    m_Unlink,
    s_unlink,
    PackedArrayInit(1)
      .append(filename)
      .toArray(),
    invoked
  );
  if (invoked && (ret.toBoolean() == true)) {
    return true;
  }

  return false;
}

bool UserFile::rename(const String& oldname, const String& newname) {
  // bool rename($oldname, $newname);
  bool invoked = false;
  Variant ret = invoke(
    m_Rename,
    s_rename,
    PackedArrayInit(2)
      .append(oldname)
      .append(newname)
      .toArray(),
    invoked
  );
  if (invoked && (ret.toBoolean() == true)) {
    return true;
  }

  return false;
}

bool UserFile::mkdir(const String& filename, int mode, int options) {
  // bool mkdir($path, $mode, $options)
  bool invoked = false;
  Variant ret = invoke(
    m_Mkdir,
    s_mkdir,
    PackedArrayInit(3)
      .append(filename)
      .append(mode)
      .append(options)
      .toArray(),
    invoked
  );
  if (invoked && (ret.toBoolean() == true)) {
    return true;
  }

  return false;
}

bool UserFile::rmdir(const String& filename, int options) {
  // bool rmdir($path, $options)
  bool invoked = false;
  Variant ret = invoke(
    m_Rmdir,
    s_rmdir,
    PackedArrayInit(2)
      .append(filename)
      .append(options)
      .toArray(),
    invoked
  );
  if (invoked && (ret.toBoolean() == true)) {
    return true;
  }

  return false;
}

bool UserFile::invokeMetadata(const Array& args, const char* funcName) {
  bool invoked = false;
  Variant ret = invoke(m_StreamMetadata, s_stream_metadata, args, invoked);
  if (!invoked) {
    raise_warning("%s(): %s::stream_metadata is not implemented!",
                  funcName, m_cls->name()->data());
    return false;
  } else if (ret.toBoolean() == true) {
    return true;
  }
  return false;
}

bool UserFile::touch(const String& path, int64_t mtime, int64_t atime) {
  if (atime == 0) {
    atime = mtime;
  }

  return invokeMetadata(
    PackedArrayInit(3)
      .append(path)
      .append(1) // STREAM_META_TOUCH
      .append(atime ? make_packed_array(mtime, atime) : Array::Create())
      .toArray(),
    "touch");
}

bool UserFile::chmod(const String& path, int64_t mode) {
  return invokeMetadata(
    PackedArrayInit(3)
      .append(path)
      .append(6) // STREAM_META_ACCESS
      .append(mode)
      .toArray(),
    "chmod");
}

bool UserFile::chown(const String& path, int64_t uid) {
  return invokeMetadata(
    PackedArrayInit(3)
      .append(path)
      .append(3) // STREAM_META_OWNER
      .append(uid)
      .toArray(),
    "chown");
}

bool UserFile::chown(const String& path, const String& uid) {
  return invokeMetadata(
    PackedArrayInit(3)
      .append(path)
      .append(2) // STREAM_META_OWNER_NAME
      .append(uid)
      .toArray(),
    "chown");
}

bool UserFile::chgrp(const String& path, int64_t gid) {
  return invokeMetadata(
    PackedArrayInit(3)
      .append(path)
      .append(5) // STREAM_META_GROUP
      .append(gid)
      .toArray(),
      "chgrp");
}

bool UserFile::chgrp(const String& path, const String& gid) {
  return invokeMetadata(
    PackedArrayInit(3)
      .append(path)
      .append(4) // STREAM_META_GROUP_NAME
      .append(gid)
      .toArray(),
    "chgrp");
}

///////////////////////////////////////////////////////////////////////////////
}
