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

  explicit UserFile(Class *cls,
                    const req::ptr<StreamContext>& context = nullptr);
  virtual ~UserFile();

  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  int fd() const override;

  bool open(const String& filename, const String& mode) override {
    return openImpl(filename, mode, 0);
  }
  bool openImpl(const String& filename, const String& mode, int options);
  bool close() override;
  int64_t readImpl(char *buffer, int64_t length) override;
  int64_t writeImpl(const char *buffer, int64_t length) override;
  bool seekable() override { return m_StreamSeek || m_Call; }
  bool seek(int64_t offset, int whence = SEEK_SET) override;
  int64_t tell() override;
  bool eof() override;
  bool rewind() override { return seek(0, SEEK_SET); }
  bool flush() override;
  bool truncate(int64_t size) override;
  bool lock(int operation) override {
    bool wouldBlock = false;
    return lock(operation, wouldBlock);
  }
  bool lock(int operation, bool &wouldBlock) override;
  bool stat(struct stat* buf) override;

  Object await(uint16_t events, double timeout) override {
    SystemLib::throwExceptionObject(
      "Userstreams do not support awaiting");
  }

  Variant getWrapperMetaData() override { return Variant(m_obj); }

  int access(const String& path, int mode);
  int lstat(const String& path, struct stat* buf);
  int stat(const String& path, struct stat* buf);
  bool unlink(const String& path);
  bool rename(const String& oldname, const String& newname);
  bool mkdir(const String& path, int mode, int options);
  bool rmdir(const String& path, int options);
  bool touch(const String& path, int64_t mtime, int64_t atime);
  bool chmod(const String& path, int64_t mode);
  bool chown(const String& path, int64_t uid);
  bool chown(const String& path, const String& uid);
  bool chgrp(const String& path, int64_t gid);
  bool chgrp(const String& path, const String& gid);

private:
  int urlStat(const String& path, struct stat* stat_sb, int flags = 0);
  bool flushImpl(bool strict);
  bool invokeMetadata(const Array& args, const char* funcName);
  Resource invokeCast(int castas);

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
  const Func* m_StreamCast;
  const Func* m_UrlStat;
  const Func* m_Unlink;
  const Func* m_Rename;
  const Func* m_Mkdir;
  const Func* m_Rmdir;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_USER_FILE_H
