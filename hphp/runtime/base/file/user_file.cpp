/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/file/user_file.h>
#include <runtime/ext/ext_function.h>
#include <runtime/vm/instance.h>
#include <runtime/vm/translator/translator-inline.h>

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(UserFile)
///////////////////////////////////////////////////////////////////////////////

StaticString UserFile::s_class_name("UserFile");
StaticString s_stream_open("stream_open");
StaticString s_stream_close("stream_close");
StaticString s_stream_read("stream_read");
StaticString s_stream_write("stream_write");
StaticString s_stream_seek("stream_seek");
StaticString s_stream_tell("stream_tell");
StaticString s_stream_eof("stream_eof");
StaticString s_stream_flush("stream_flush");
StaticString s_stream_lock("stream_lock");
StaticString s_call("__call");

///////////////////////////////////////////////////////////////////////////////

UserFile::UserFile(VM::Class *cls, int options /*= 0 */,
                   CVarRef context /*= null */) :
                   m_cls(cls), m_options(options) {
  VM::Transl::VMRegAnchor _;
  const VM::Func *ctor;
  if (MethodLookup::MethodFoundWithThis !=
      g_vmContext->lookupCtorMethod(ctor, cls)) {
    throw InvalidArgumentException(0, "Unable to call %s's constructor",
                                   cls->name()->data());
  }

  m_obj = VM::Instance::newInstance(cls);
  m_obj.o_set("context", context);
  Variant ret;
  g_vmContext->invokeFunc(ret.asTypedValue(), ctor,
                          Array::Create(), m_obj.get());

  m_StreamOpen  = lookupMethod(s_stream_open.get());
  m_StreamClose = lookupMethod(s_stream_close.get());
  m_StreamRead  = lookupMethod(s_stream_read.get());
  m_StreamWrite = lookupMethod(s_stream_write.get());
  m_StreamSeek  = lookupMethod(s_stream_seek.get());
  m_StreamTell  = lookupMethod(s_stream_tell.get());
  m_StreamEof   = lookupMethod(s_stream_eof.get());
  m_StreamFlush = lookupMethod(s_stream_flush.get());
  m_StreamLock  = lookupMethod(s_stream_lock.get());

  m_Call        = lookupMethod(s_call.get());
}

UserFile::~UserFile() {
}

const VM::Func* UserFile::lookupMethod(const StringData* name) {
  const VM::Func *f = m_cls->lookupMethod(name);
  if (!f) return nullptr;

  if (f->attrs() & AttrStatic) {
    throw InvalidArgumentException(0, "%s::%s() must not be declared static",
                                   m_cls->name()->data(), name->data());
  }
  return f;
}

///////////////////////////////////////////////////////////////////////////////

Variant UserFile::invoke(const VM::Func *func, CStrRef name,
                         CArrRef args, bool &success) {
  VM::Transl::VMRegAnchor _;

  // Assume failure
  success = false;

  // Public method, no private ancestor, no need for further checks (common)
  if (func &&
      !(func->attrs() & (AttrPrivate|AttrProtected|AttrAbstract)) &&
      !func->hasPrivateAncestor()) {
    Variant ret;
    g_vmContext->invokeFunc(ret.asTypedValue(), func, args, m_obj.get());
    success = true;
    return ret;
  }

  // No explicitly defined function, no __call() magic method
  // Give up.
  if (!func && !m_Call) {
    return uninit_null();
  }

  switch(g_vmContext->lookupObjMethod(func, m_cls, name.get())) {
    case MethodLookup::MethodFoundWithThis:
    {
      Variant ret;
      g_vmContext->invokeFunc(ret.asTypedValue(), func, args, m_obj.get());
      success = true;
      return ret;
    }

    case MethodLookup::MagicCallFound:
    {
      Variant ret;
      g_vmContext->invokeFunc(ret.asTypedValue(), func,
                              CREATE_VECTOR2(name, args), m_obj.get());
      success = true;
      return ret;
    }

    case MethodLookup::MethodNotFound:
      // There's a method somewhere in the heirarchy, but none
      // which are accessible.
      /* fallthrough */
    case MethodLookup::MagicCallStaticFound:
      // We're not calling staticly, so this result is unhelpful
      // Also, it's never produced by lookupObjMethod, so it'll
      // never happen, but we must handle all enums
      return uninit_null();

    case MethodLookup::MethodFoundNoThis:
      // Should never happen (Attr::Static check in ctor)
      assert(false);
      raise_error("%s::%s() must not be declared static",
                  m_cls->name()->data(), name.data());
      return uninit_null();
  }

  NOT_REACHED();
  return uninit_null();
}

///////////////////////////////////////////////////////////////////////////////

bool UserFile::open(CStrRef filename, CStrRef mode) {
  // bool stream_open($path, $mode, $options, &$opened_path)
  bool success = false;
  Variant opened_path;
  Variant ret = invoke(m_StreamOpen, s_stream_open, CREATE_VECTOR4(
                       filename, mode, m_options, ref(opened_path)), success);
  if (success && (ret.toBoolean() == true)) {
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
                      CREATE_VECTOR1(length), success);
  if (!success) {
    raise_warning("%s::stream_read is not implemented",
                  m_cls->name()->data());
    return 0;
  }

  int64_t didread = str.size();
  if (didread > length) {
    raise_warning("%s::stream_read - read %ld bytes more data than requested "
                  "(%ld read, %ld max) - excess data will be lost",
                  m_cls->name()->data(), (long)(didread - length),
                  (long)didread, (long)length);
    didread = length;
  }

  memcpy(buffer, str.data(), didread);
  return didread;
}

int64_t UserFile::writeImpl(const char *buffer, int64_t length) {
  // stream_write($data)
  bool success = false;
  int64_t didWrite = invoke(m_StreamWrite, s_stream_write,
                          CREATE_VECTOR1(String(buffer, length, CopyString)),
                          success);
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
  }

  return didWrite;
}

///////////////////////////////////////////////////////////////////////////////

bool UserFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  // bool stream_seek($offset, $whence)
  bool success = false;
  bool sought  = invoke(m_StreamSeek, s_stream_seek,
                        CREATE_VECTOR2(offset, whence), success);
  return success ? sought : false;
}

int64_t UserFile::tell() {
  // int stream_tell()
  bool success = false;
  Variant ret = invoke(m_StreamTell, s_stream_tell, Array::Create(), success);
  if (!success) {
    raise_warning("%s::stream_tell is not implemented!", m_cls->name()->data());
    return -1;
  }
  return ret.isInteger() ? ret.toInt64() : -1;
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

bool UserFile::lock(int operation, bool &wouldBlock) {
  int64_t op = 0;
  if (operation & LOCK_NB) {
    op |= k_LOCK_NB;
  }
  switch (operation & ~LOCK_NB) {
    case LOCK_SH: op |= k_LOCK_SH; break;
    case LOCK_EX: op |= k_LOCK_EX; break;
    case LOCK_UN: op |= k_LOCK_UN; break;
  }

  // bool stream_lock(int $operation)
  bool success = false;
  Variant ret = invoke(m_StreamLock, s_stream_lock,
                       CREATE_VECTOR1(op), success);
  if (!success) {
    if (operation) {
      raise_warning("%s::stream_lock is not implemented!",
                    m_cls->name()->data());
    }
    return false;
  }
  return ret.isBoolean() ? ret.toBoolean() : false;
}

///////////////////////////////////////////////////////////////////////////////
}
