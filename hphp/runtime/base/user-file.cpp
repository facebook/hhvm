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

#include "hphp/runtime/base/user-file.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

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
StaticString s_url_stat("url_stat");

///////////////////////////////////////////////////////////////////////////////

UserFile::UserFile(Class *cls, CVarRef context /*= null */) :
                   UserFSNode(cls), m_opened(false) {
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
  m_UrlStat     = lookupMethod(s_url_stat.get());
  m_isLocal = true;
}

UserFile::~UserFile() {
  if (m_opened) {
    close();
  }
}

void UserFile::sweep() {
  File::sweep();
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

bool UserFile::openImpl(const String& filename, const String& mode,
                        int options) {
  // bool stream_open($path, $mode, $options, &$opened_path)
  bool success = false;
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
    success
  );
  if (success && (ret.toBoolean() == true)) {
    m_opened = true;
    return true;
  }

  raise_warning("\"%s::stream_open\" call failed", m_cls->name()->data());
  return false;
}

bool UserFile::close() {
  // PHP's streams layer explicitly flushes on close
  // Mimick that for user-wrappers by pushing the flush here
  // without impacting other HPHP stream types.
  flush();

  // void stream_close()
  invoke(m_StreamClose, s_stream_close, Array::Create());
  return true;
}

///////////////////////////////////////////////////////////////////////////////

int64_t UserFile::readImpl(char *buffer, int64_t length) {
  // String stread_read($count)
  bool success = false;
  String str = invoke(m_StreamRead, s_stream_read,
                      make_packed_array(length), success);
  if (!success) {
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
    bool success = false;
    int64_t didWrite = invoke(
      m_StreamWrite,
      s_stream_write,
      make_packed_array(String(buffer, length, CopyString)),
      success
    ).toInt64();
    if (!success) {
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
  // bool stream_seek($offset, $whence)
  bool success = false;
  bool sought  = invoke(
    m_StreamSeek, s_stream_seek, make_packed_array(offset, whence), success
  ).toBoolean();
  if (!success) {
    always_assert("No seek method? But I found one earlier?");
  }

  // If the userland code failed to update, do it on our buffers instead
  if (!sought) {
    if (whence == SEEK_CUR) {
      m_position += offset;
    } else if (whence == SEEK_SET) {
      m_position = offset;
    }
    return true;
  }

  // int stream_tell()
  Variant ret = invoke(m_StreamTell, s_stream_tell, Array::Create(), success);
  if (!success) {
    raise_warning("%s::stream_tell is not implemented!", m_cls->name()->data());
    return false;
  }
  m_position = ret.isInteger() ? ret.toInt64() : -1;
  return true;
}

int64_t UserFile::tell() {
  return m_position;
}

bool UserFile::eof() {
  // If there's data in the read buffer, then we're clearly not EOF
  if ((m_writepos - m_readpos) > 0) {
    return false;
  }

  // bool stream_eof()
  bool success = false;
  Variant ret = invoke(m_StreamEof, s_stream_eof, Array::Create(), success);
  if (!success) {
    return false;
  }
  return ret.isBoolean() ? ret.toBoolean() : true;
}

bool UserFile::flush() {
  // bool stream_flush()
  bool success = false;
  Variant ret = invoke(m_StreamFlush, s_stream_flush,
                       Array::Create(), success);
  if (!success) {
    return false;
  }
  return ret.isBoolean() ? ret.toBoolean() : false;
}

bool UserFile::truncate(int64_t size) {
  // bool stream_truncate()
  bool success = false;
  Variant ret = invoke(m_StreamTruncate, s_stream_truncate,
                       make_packed_array(size), success);
  if (!success) {
    return false;
  }
  return ret.isBoolean() ? ret.toBoolean() : false;
}

bool UserFile::lock(int operation, bool &wouldBlock) {
  int64_t op = 0;
  if (operation & LOCK_NB) {
    op |= LOCK_NB;
  }
  switch (operation & ~LOCK_NB) {
    case LOCK_SH: op |= LOCK_SH; break;
    case LOCK_EX: op |= LOCK_EX; break;
    case LOCK_UN: op |= LOCK_UN; break;
  }

  // bool stream_lock(int $operation)
  bool success = false;
  Variant ret = invoke(m_StreamLock, s_stream_lock,
                       make_packed_array(op), success);
  if (!success) {
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
int UserFile::statImpl(const String& path, struct stat* stat_sb,
                       int flags /* = 0 */) {
  // array url_stat ( string $path , int $flags )
  bool success = false;
  Variant ret = invoke(m_UrlStat, s_url_stat,
                       make_packed_array(path, flags), success);
  if (!ret.isArray()) {
    return -1;
  }
  auto a = ret.toArray();
  stat_sb->st_dev = a.lval(s_dev).toInt64();
  stat_sb->st_ino = a.lval(s_ino).toInt64();
  stat_sb->st_mode = a.lval(s_mode).toInt64();
  stat_sb->st_nlink = a.lval(s_nlink).toInt64();
  stat_sb->st_uid = a.lval(s_uid).toInt64();
  stat_sb->st_gid = a.lval(s_gid).toInt64();
  stat_sb->st_rdev = a.lval(s_rdev).toInt64();
  stat_sb->st_size = a.lval(s_size).toInt64();
  stat_sb->st_atime = a.lval(s_atime).toInt64();
  stat_sb->st_mtime = a.lval(s_mtime).toInt64();
  stat_sb->st_ctime = a.lval(s_ctime).toInt64();
  stat_sb->st_blksize = a.lval(s_blksize).toInt64();
  stat_sb->st_blocks = a.lval(s_blocks).toInt64();
  return 0;
}

extern const int64_t k_STREAM_URL_STAT_QUIET;
int UserFile::access(const String& path, int mode) {
  struct stat buf;
  auto ret = statImpl(path, &buf, k_STREAM_URL_STAT_QUIET);
  if (ret < 0 || mode == F_OK) {
    return ret;
  }
  return buf.st_mode & mode ? 0 : -1;
}

extern const int64_t k_STREAM_URL_STAT_LINK;
int UserFile::lstat(const String& path, struct stat* buf) {
  return statImpl(path, buf, k_STREAM_URL_STAT_LINK);
}

int UserFile::stat(const String& path, struct stat* buf) {
  return statImpl(path, buf);
}

///////////////////////////////////////////////////////////////////////////////
}
