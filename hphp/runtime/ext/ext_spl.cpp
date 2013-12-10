/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_spl.h"
#include "hphp/runtime/ext/ext_math.h"
#include "hphp/runtime/ext/ext_class.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/ext/ext_file.h"

#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/base/glob-stream-wrapper.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_spl_autoload("spl_autoload"),
  s_spl_autoload_call("spl_autoload_call"),
  s_default_extensions(".inc,.php"),
  s_rewind("rewind"),
  s_valid("valid"),
  s_next("next"),
  s_current("current"),
  s_key("key"),
  s_getIterator("getIterator"),
  s_directory_iterator("DirectoryIterator");

const StaticString spl_classes[] = {
  StaticString("AppendIterator"),
  StaticString("ArrayIterator"),
  StaticString("ArrayObject"),
  StaticString("BadFunctionCallException"),
  StaticString("BadMethodCallException"),
  StaticString("CachingIterator"),
  StaticString("Countable"),
  s_directory_iterator,
  StaticString("DomainException"),
  StaticString("EmptyIterator"),
  StaticString("FilesystemIterator"),
  StaticString("FilterIterator"),
  StaticString("GlobIterator"),
  StaticString("InfiniteIterator"),
  StaticString("InvalidArgumentException"),
  StaticString("IteratorIterator"),
  StaticString("LengthException"),
  StaticString("LimitIterator"),
  StaticString("LogicException"),
  StaticString("MultipleIterator"),
  StaticString("NoRewindIterator"),
  StaticString("OuterIterator"),
  StaticString("OutOfBoundsException"),
  StaticString("OutOfRangeException"),
  StaticString("OverflowException"),
  StaticString("ParentIterator"),
  StaticString("RangeException"),
  StaticString("RecursiveArrayIterator"),
  StaticString("RecursiveCachingIterator"),
  StaticString("RecursiveDirectoryIterator"),
  StaticString("RecursiveFilterIterator"),
  StaticString("RecursiveIterator"),
  StaticString("RecursiveIteratorIterator"),
  StaticString("RecursiveRegexIterator"),
  StaticString("RecursiveTreeIterator"),
  StaticString("RegexIterator"),
  StaticString("RuntimeException"),
  StaticString("SeekableIterator"),
  StaticString("SplDoublyLinkedList"),
  StaticString("SplFileInfo"),
  StaticString("SplFileObject"),
  StaticString("SplFixedArray"),
  StaticString("SplHeap"),
  StaticString("SplMinHeap"),
  StaticString("SplMaxHeap"),
  StaticString("SplObjectStorage"),
  StaticString("SplObserver"),
  StaticString("SplPriorityQueue"),
  StaticString("SplQueue"),
  StaticString("SplStack"),
  StaticString("SplSubject"),
  StaticString("SplTempFileObject"),
  StaticString("UnderflowException"),
  StaticString("UnexpectedValueException"),
};

Array f_spl_classes() {
  const size_t num_classes = sizeof(spl_classes) / sizeof(spl_classes[0]);
  ArrayInit ret(num_classes);
  for (size_t i = 0; i < num_classes; ++i) {
    ret.set(spl_classes[i], spl_classes[i]);
  }
  return ret.create();
}

void throw_spl_exception(const char *fmt, ...) ATTRIBUTE_PRINTF(1,2);
void throw_spl_exception(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string msg;
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  throw Object(SystemLib::AllocExceptionObject(Variant(msg)));
}

static bool s_inited = false;
static int64_t s_hash_mask_handle = 0;
static Mutex s_mutex;

String f_spl_object_hash(CObjRef obj) {
  if (!s_inited) {
    Lock lock(s_mutex);
    if (!s_inited) {
      f_mt_srand();
      s_hash_mask_handle |= f_mt_rand(); s_hash_mask_handle <<= 16;
      s_hash_mask_handle |= f_mt_rand(); s_hash_mask_handle <<= 16;
      s_hash_mask_handle |= f_mt_rand(); s_hash_mask_handle <<= 16;
      s_hash_mask_handle |= f_mt_rand();
      s_inited = true;
    }
  }

  char buf[33];
  snprintf(buf, sizeof(buf), "%032" PRIx64,
           s_hash_mask_handle ^ (int64_t)obj.get());
  return String(buf, CopyString);
}

int64_t f_hphp_object_pointer(CObjRef obj) { return (int64_t)obj.get();}

Variant f_hphp_get_this() {
  return g_vmContext->getThis();
}

Variant f_class_implements(CVarRef obj, bool autoload /* = true */) {
  Class* cls;
  if (obj.isString()) {
    cls = Unit::getClass(obj.getStringData(), autoload);
    if (!cls) {
      return false;
    }
  } else if (obj.isObject()) {
    cls = obj.getObjectData()->getVMClass();
  } else {
    return false;
  }
  Array ret(Array::Create());
  const Class::InterfaceMap& ifaces = cls->allInterfaces();
  for (int i = 0, size = ifaces.size(); i < size; i++) {
    ret.set(ifaces[i]->nameRef(), ifaces[i]->nameRef());
  }
  return ret;
}

Variant f_class_parents(CVarRef obj, bool autoload /* = true */) {
  Class* cls;
  if (obj.isString()) {
    cls = Unit::getClass(obj.getStringData(), autoload);
    if (!cls) {
      return false;
    }
  } else if (obj.isObject()) {
    cls = obj.getObjectData()->getVMClass();
  } else {
    return false;
  }
  Array ret(Array::Create());
  for (cls = cls->parent(); cls; cls = cls->parent()) {
    auto& clsName = cls->nameRef();
    ret.set(clsName, clsName);
  }
  return ret;
}

Variant f_class_uses(CVarRef obj, bool autoload /* = true */) {
  Class* cls;
  if (obj.isString()) {
    cls = Unit::getClass(obj.getStringData(), autoload);
    if (!cls) {
      return false;
    }
  } else if (obj.isObject()) {
    cls = obj.getObjectData()->getVMClass();
  } else {
    return false;
  }
  Array ret(Array::Create());
  for (auto& elem : cls->usedTraits()) {
    auto& traitName = elem->nameRef();
    ret.set(traitName, traitName);
  }
  return ret;
}

Object get_traversable_object_iterator(CVarRef obj) {
  if (!obj.instanceof(SystemLib::s_TraversableClass)) {
    raise_error("Argument must implement interface Traversable");
  }

  bool isIteratorAggregate;
  Object itObj = obj.getObjectData()
    ->iterableObject(isIteratorAggregate, true);

  if (!isIteratorAggregate) {
    if (obj.instanceof(SystemLib::s_IteratorAggregateClass)) {
      raise_error("Objects returned by getIterator() must be traversable or "
                  "implement interface Iterator");
    } else {
      raise_error(
        "Class %s must implement interface Traversable as part of either "
        "Iterator or IteratorAggregate",
        obj.toObject()->o_getClassName()->data()
      );
    }
  }

  return itObj;
}

Variant f_iterator_apply(CVarRef obj, CVarRef func,
                         CArrRef params /* = null_array */) {
  Object pobj = get_traversable_object_iterator(obj);
  pobj->o_invoke_few_args(s_rewind, 0);
  int64_t count = 0;
  while (same(pobj->o_invoke_few_args(s_valid, 0), true)) {
    if (!same(vm_call_user_func(func, params), true)) {
      break;
    }
    ++count;
    pobj->o_invoke_few_args(s_next, 0);
  }
  return count;
}

Variant f_iterator_count(CVarRef obj) {
  Object pobj = get_traversable_object_iterator(obj);
  pobj->o_invoke_few_args(s_rewind, 0);
  int64_t count = 0;
  while (same(pobj->o_invoke_few_args(s_valid, 0), true)) {
    ++count;
    pobj->o_invoke_few_args(s_next, 0);
  }
  return count;
}

Variant f_iterator_to_array(CVarRef obj, bool use_keys /* = true */) {
  Object pobj = get_traversable_object_iterator(obj);
  Array ret(Array::Create());

  pobj->o_invoke_few_args(s_rewind, 0);
  while (same(pobj->o_invoke_few_args(s_valid, 0), true)) {
    Variant val = pobj->o_invoke_few_args(s_current, 0);
    if (use_keys) {
      Variant key = pobj->o_invoke_few_args(s_key, 0);
      ret.set(key, val);
    } else {
      ret.append(val);
    }
    pobj->o_invoke_few_args(s_next, 0);
  }
  return ret;
}

bool f_spl_autoload_register(CVarRef autoload_function /* = null_variant */,
                             bool throws /* = true */,
                             bool prepend /* = false */) {
  if (same(autoload_function, s_spl_autoload_call)) {
    if (throws) {
      throw_spl_exception("Function spl_autoload_call()"
                      "cannot be registered");
    }
    return false;
  }
  CVarRef func = autoload_function.isNull() ?
                 s_spl_autoload : autoload_function;
  bool res = AutoloadHandler::s_instance->addHandler(func, prepend);
  if (!res && throws) {
    throw_spl_exception("Invalid autoload_function specified");
  }
  return res;
}

bool f_spl_autoload_unregister(CVarRef autoload_function) {
  if (same(autoload_function, s_spl_autoload_call)) {
    AutoloadHandler::s_instance->removeAllHandlers();
  } else {
    AutoloadHandler::s_instance->removeHandler(autoload_function);
  }
  return true;
}

Variant f_spl_autoload_functions() {
  CArrRef handlers = AutoloadHandler::s_instance->getHandlers();
  if (handlers.isNull())
    return false;
  else
    return handlers.values();
}

void f_spl_autoload_call(const String& class_name) {
  AutoloadHandler::s_instance->invokeHandler(class_name, true);
}

namespace {
class ExtensionList : public RequestEventHandler {
public:
  virtual void requestInit() {
    extensions = make_packed_array(String(".inc"), String(".php"));
  }
  virtual void requestShutdown() {
    extensions.reset();
  }

  Array extensions;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(ExtensionList, s_extension_list);
}

String f_spl_autoload_extensions(const String& file_extensions /* = null_string */) {
  if (!file_extensions.isNull()) {
    s_extension_list->extensions = StringUtil::Explode(file_extensions, ",")
                                   .toArray();
    return file_extensions;
  }
  return StringUtil::Implode(s_extension_list->extensions, ",");
}

void f_spl_autoload(const String& class_name,
                    const String& file_extensions /* = null_string */) {
  Array ext = file_extensions.isNull()
              ? s_extension_list->extensions
              : StringUtil::Explode(file_extensions, ",").toArray();
  String lClass = f_strtolower(class_name);
  bool found = false;
  for (ArrayIter iter(ext); iter; ++iter) {
    String fileName = lClass + iter.second().toString();
    include(fileName, true, "", false);
    if (f_class_exists(class_name, false)) {
      found = true;
      break;
    }
  }

  if (!found && !AutoloadHandler::s_instance->isRunning()) {
    throw_spl_exception("Class %s could not be loaded", class_name.c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////

template <class T>
static T* getDir(const Object& dir_iter) {
  static_assert(std::is_base_of<Directory, T>::value,
                "Only cast to directories");
  auto dir = dir_iter->o_realProp("dir", 0, s_directory_iterator);
  return dir->asCResRef().getTyped<T>();
}

static Variant HHVM_METHOD(DirectoryIterator, hh_readdir) {
  auto dir = getDir<Directory>(this_);

  if (auto array_dir = dynamic_cast<ArrayDirectory*>(dir)) {
    auto prop = this_->o_realProp("dirName", 0, s_directory_iterator);
    *prop = array_dir->path();
  }

  return f_readdir(Resource(dir));
}

static int64_t HHVM_METHOD(GlobIterator, count) {
  return getDir<ArrayDirectory>(this_)->size();
}

///////////////////////////////////////////////////////////////////////////////

static class SPLExtension : public Extension {
 public:
  SPLExtension() : Extension("SPL") { }
  virtual void moduleLoad(Hdf config) {
    HHVM_ME(DirectoryIterator, hh_readdir);
    HHVM_ME(GlobIterator, count);
  }
} s_SPL_extension;

///////////////////////////////////////////////////////////////////////////////
}
