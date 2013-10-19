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

#ifndef HPHP_USER_FILE_H
#define HPHP_USER_FILE_H

#include "hphp/runtime/base/file.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class UserFile : public File {
public:
  DECLARE_RESOURCE_ALLOCATION(UserFile);

  explicit UserFile(Class *cls, int options = 0,
                    CVarRef context = uninit_null());
  virtual ~UserFile();

  // overriding ResourceData
  const String& o_getClassNameHook() const { return classnameof(); }

  virtual bool open(const String& filename, const String& mode);
  virtual bool close();
  virtual int64_t readImpl(char *buffer, int64_t length);
  virtual int getc() {
    char buf[1];
    if (readImpl(buf, 1) == 1) {
      return buf[0];
    }
    return 0;
  }
  virtual int64_t writeImpl(const char *buffer, int64_t length);
  virtual bool seekable() { return m_StreamSeek || m_Call; }
  virtual bool seek(int64_t offset, int whence = SEEK_SET);
  virtual int64_t tell();
  virtual bool eof();
  virtual bool rewind() { return seek(0, SEEK_SET); }
  virtual bool flush();
  virtual bool lock(int operation) {
    bool wouldBlock = false;
    return lock(operation, wouldBlock);
  }
  virtual bool lock(int operation, bool &wouldBlock);

protected:
  Class *m_cls;
  int m_options;
  Object m_obj;

  Variant invoke(const Func *func, const String& name, CArrRef args,
                 bool &success);
  Variant invoke(const Func *func, const String& name, CArrRef args) {
    bool success;
    return invoke(func, name, args, success);
  }

  const Func* lookupMethod(const StringData* name);

  const Func* m_StreamOpen;
  const Func* m_StreamClose;
  const Func* m_StreamRead;
  const Func* m_StreamWrite;
  const Func* m_StreamSeek;
  const Func* m_StreamTell;
  const Func* m_StreamEof;
  const Func* m_StreamFlush;
  const Func* m_StreamLock;

  const Func* m_Call;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_USER_FILE_H
