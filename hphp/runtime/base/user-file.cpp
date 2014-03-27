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
StaticString s_stream_stat("stream_stat");
StaticString s_url_stat("url_stat");
StaticString s_unlink("unlink");
StaticString s_rename("rename");
StaticString s_mkdir("mkdir");
StaticString s_rmdir("rmdir");

///////////////////////////////////////////////////////////////////////////////

UserFile::UserFile(Class *cls, const Variant& context /*= null */) : UserFSNode(cls) {
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
  m_isLocal = true;

  // UserFile, to match Zend, should not call stream_close() unless it was ever
  // opened. This is a bit of a misuse of this field but the API doesn't allow
  // one direct access to an not-yet-opened stream resource so it should be
  // safe.
  m_closed = true;
}

UserFile::~UserFile() {
  if (!m_closed) {
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
    m_closed = false;
    return true;
  }

  raise_warning("\"%s::stream_open\" call failed", m_cls->name()->data());
  return false;
}

bool UserFile::close() {
  // fclose() should prevent this from being called on a closed stream
  assert(!m_closed);

  // PHP's streams layer explicitly flushes on close
  // Mimick that for user-wrappers by pushing the flush here
  // without impacting other HPHP stream types.
  bool ret = flushImpl(false) || !RuntimeOption::CheckFlushOnUserClose;

  // void stream_close()
  invoke(m_StreamClose, s_stream_close, Array::Create());
  m_closed = true;
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
      0 <= m_readpos + offset &&
           m_readpos + offset < m_writepos) {
    m_readpos += offset;
    m_position += offset;
    return true;
  } else if (whence == SEEK_SET &&
             0 <= offset - m_position + m_readpos &&
                  offset - m_position + m_readpos < m_writepos) {
    m_readpos = offset - m_position + m_readpos;
    m_position = offset;
    return true;
  } else {
    if (whence == SEEK_CUR) {
      offset += m_readpos - m_writepos;
    }
    m_readpos = 0;
    m_writepos = 0;
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
    op |= LOCK_NB;
  }
  switch (operation & ~LOCK_NB) {
    case LOCK_SH: op |= LOCK_SH; break;
    case LOCK_EX: op |= LOCK_EX; break;
    case LOCK_UN: op |= LOCK_UN; break;
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

extern const int64_t k_STREAM_URL_STAT_QUIET;
int UserFile::access(const String& path, int mode) {
  struct stat buf;
  auto ret = urlStat(path, &buf, k_STREAM_URL_STAT_QUIET);
  if (ret < 0 || mode == F_OK) {
    return ret;
  }
  return buf.st_mode & mode ? 0 : -1;
}

extern const int64_t k_STREAM_URL_STAT_LINK;
int UserFile::lstat(const String& path, struct stat* buf) {
  return urlStat(path, buf, k_STREAM_URL_STAT_LINK);
}

int UserFile::stat(const String& path, struct stat* buf) {
  return urlStat(path, buf);
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

  raise_warning("\"%s::unlink\" call failed", m_cls->name()->data());
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

  raise_warning("\"%s::rename\" call failed", m_cls->name()->data());
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

  raise_warning("\"%s::mkdir\" call failed", m_cls->name()->data());
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

  raise_warning("\"%s::rmdir\" call failed", m_cls->name()->data());
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
