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

#ifndef HPHP_USER_FILE_H
#define HPHP_USER_FILE_H

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/user-fs-node.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class UserFile : public File, public UserFSNode {
public:
  DECLARE_RESOURCE_ALLOCATION(UserFile);

  explicit UserFile(Class *cls, const Variant& context = uninit_null());
  virtual ~UserFile();

  // overriding ResourceData
  const String& o_getClassNameHook() const { return classnameof(); }

  virtual bool open(const String& filename, const String& mode) {
    return openImpl(filename, mode, 0);
  }
  bool openImpl(const String& filename, const String& mode, int options);
  virtual bool close();
  virtual int64_t readImpl(char *buffer, int64_t length);
  virtual int64_t writeImpl(const char *buffer, int64_t length);
  virtual bool seekable() { return m_StreamSeek || m_Call; }
  virtual bool seek(int64_t offset, int whence = SEEK_SET);
  virtual int64_t tell();
  virtual bool eof();
  virtual bool rewind() { return seek(0, SEEK_SET); }
  virtual bool flush();
  virtual bool truncate(int64_t size);
  virtual bool lock(int operation) {
    bool wouldBlock = false;
    return lock(operation, wouldBlock);
  }
  virtual bool lock(int operation, bool &wouldBlock);
  virtual bool stat(struct stat* buf);

  int access(const String& path, int mode);
  int lstat(const String& path, struct stat* buf);
  int stat(const String& path, struct stat* buf);
  bool unlink(const String& path);
  bool rename(const String& oldname, const String& newname);
  bool mkdir(const String& path, int mode, int options);
  bool rmdir(const String& path, int options);
  bool touch(const String& path, int64_t mtime, int64_t atime);

private:
  int urlStat(const String& path, struct stat* stat_sb, int flags = 0);
  bool flushImpl(bool strict);

protected:
  const Func* m_StreamOpen;
  const Func* m_StreamClose;
  const Func* m_StreamRead;
  const Func* m_StreamWrite;
  const Func* m_StreamSeek;
  const Func* m_StreamTell;
  const Func* m_StreamEof;
  const Func* m_StreamFlush;
  const Func* m_StreamTruncate;
  const Func* m_StreamLock;
  const Func* m_StreamStat;
  const Func* m_StreamMetadata;
  const Func* m_UrlStat;
  const Func* m_Unlink;
  const Func* m_Rename;
  const Func* m_Mkdir;
  const Func* m_Rmdir;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_USER_FILE_H
