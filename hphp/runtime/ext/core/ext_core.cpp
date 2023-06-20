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

#include "hphp/runtime/ext/core/ext_core.h"

#include "hphp/runtime/ext/extension.h"

#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/ext/std/ext_std_file.h"

namespace HPHP {

#define SYSTEM_CLASS_STRING(cls, prefix, namespace)     \
  const StaticString s_##prefix##cls(#namespace #cls);
SYSTEMLIB_CLASSES(SYSTEM_CLASS_STRING)
#undef SYSTEM_CLASS_STRING

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_dir("dir"),
  s_dirName("dirName");

template <class T>
static req::ptr<T> getDir(const Object& dir_iter) {
  static_assert(std::is_base_of<Directory, T>::value,
                "Only cast to directories");
  assertx(SystemLib::s_DirectoryIteratorClass);
  auto const dir = dir_iter->getProp(
    MemberLookupContext(SystemLib::s_DirectoryIteratorClass,
                        SystemLib::s_DirectoryIteratorClass->moduleName()),
    s_dir.get());
  assertx(dir.is_set());
  assertx(dir.type() == KindOfResource);
  return req::ptr<T>(static_cast<T*>(dir.val().pres->data()));
}

static Variant HHVM_METHOD(DirectoryIterator, hh_readdir) {
  auto dir = getDir<Directory>(ObjNR(this_).asObject());

  if (auto array_dir = dyn_cast<ArrayDirectory>(dir)) {
    auto const path = array_dir->path();
    assertx(SystemLib::s_DirectoryIteratorClass);
    this_->setProp(
      MemberLookupContext(SystemLib::s_DirectoryIteratorClass,
                          SystemLib::s_DirectoryIteratorClass->moduleName()),
      s_dirName.get(), path.asTypedValue());
  }

  return HHVM_FN(readdir)(Resource(dir));
}

static int64_t HHVM_METHOD(GlobIterator, count) {
  return getDir<ArrayDirectory>(ObjNR(this_).asObject())->size();
}

void CoreExtension::moduleInit() {
  initClosure();

  HHVM_ME(DirectoryIterator, hh_readdir);
  HHVM_ME(GlobIterator, count);

  loadSystemlib();

  SystemLib::s_nullFunc =
    Func::lookup(makeStaticString("__SystemLib\\__86null"));

#define INIT_SYSTEMLIB_CLASS_FIELD(cls, prefix, ...)               \
  {                                                                \
    Class *cls = NamedType::get(s_##prefix##cls.get())->clsList(); \
    assert(cls);                                                   \
    SystemLib::s_##prefix##cls ##Class = cls;                      \
  }

  // Stash a pointer to the VM Classes for stdClass, Exception,
  // pinitSentinel and resource
  SYSTEMLIB_CLASSES(INIT_SYSTEMLIB_CLASS_FIELD)

#undef INIT_SYSTEMLIB_CLASS_FIELD
}

CoreExtension s_core_extension;

}
