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

#include <runtime/base/file/file.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class UserFile : public File {
public:
  DECLARE_OBJECT_ALLOCATION(UserFile);

  UserFile(VM::Class *cls, int options = 0, CVarRef context = uninit_null());
  virtual ~UserFile();

  static StaticString s_class_name;
  // overriding ResourceData
  CStrRef o_getClassNameHook() const { return s_class_name; }

  virtual bool open(CStrRef filename, CStrRef mode);
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
  VM::Class *m_cls;
  int m_options;
  Object m_obj;

  Variant invoke(const VM::Func *func, CStrRef name, CArrRef args, bool &success);
  Variant invoke(const VM::Func *func, CStrRef name, CArrRef args) {
    bool success;
    return invoke(func, name, args, success);
  }

  const VM::Func* lookupMethod(const StringData* name);

  const VM::Func* m_StreamOpen;
  const VM::Func* m_StreamClose;
  const VM::Func* m_StreamRead;
  const VM::Func* m_StreamWrite;
  const VM::Func* m_StreamSeek;
  const VM::Func* m_StreamTell;
  const VM::Func* m_StreamEof;
  const VM::Func* m_StreamFlush;
  const VM::Func* m_StreamLock;

  const VM::Func* m_Call;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_USER_FILE_H
