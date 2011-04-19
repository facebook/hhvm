/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/ext_spl.h>
#include <runtime/ext/ext_math.h>
#include <runtime/ext/ext_class.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static StaticString s_spl_autoload("spl_autoload");
static StaticString s_spl_autoload_call("spl_autoload_call");
static StaticString s_default_extensions(".inc,.php");

#define SPL_ADD_CLASS(cls) ret.set(#cls, #cls)

Array f_spl_classes() {
  Array ret;
  SPL_ADD_CLASS(AppendIterator);
  SPL_ADD_CLASS(ArrayIterator);
  SPL_ADD_CLASS(ArrayObject);
  SPL_ADD_CLASS(BadFunctionCallException);
  SPL_ADD_CLASS(BadMethodCallException);
  SPL_ADD_CLASS(CachingIterator);
  SPL_ADD_CLASS(Countable);
  SPL_ADD_CLASS(DirectoryIterator);
  SPL_ADD_CLASS(DomainException);
  SPL_ADD_CLASS(EmptyIterator);
  SPL_ADD_CLASS(FilesystemIterator);
  SPL_ADD_CLASS(FilterIterator);
  SPL_ADD_CLASS(GlobIterator);
  SPL_ADD_CLASS(InfiniteIterator);
  SPL_ADD_CLASS(InvalidArgumentException);
  SPL_ADD_CLASS(IteratorIterator);
  SPL_ADD_CLASS(LengthException);
  SPL_ADD_CLASS(LimitIterator);
  SPL_ADD_CLASS(LogicException);
  SPL_ADD_CLASS(MultipleIterator);
  SPL_ADD_CLASS(NoRewindIterator);
  SPL_ADD_CLASS(OuterIterator);
  SPL_ADD_CLASS(OutOfBoundsException);
  SPL_ADD_CLASS(OutOfRangeException);
  SPL_ADD_CLASS(OverflowException);
  SPL_ADD_CLASS(ParentIterator);
  SPL_ADD_CLASS(RangeException);
  SPL_ADD_CLASS(RecursiveArrayIterator);
  SPL_ADD_CLASS(RecursiveCachingIterator);
  SPL_ADD_CLASS(RecursiveDirectoryIterator);
  SPL_ADD_CLASS(RecursiveFilterIterator);
  SPL_ADD_CLASS(RecursiveIterator);
  SPL_ADD_CLASS(RecursiveIteratorIterator);
  SPL_ADD_CLASS(RecursiveRegexIterator);
  SPL_ADD_CLASS(RecursiveTreeIterator);
  SPL_ADD_CLASS(RegexIterator);
  SPL_ADD_CLASS(RuntimeException);
  SPL_ADD_CLASS(SeekableIterator);
  SPL_ADD_CLASS(SplDoublyLinkedList);
  SPL_ADD_CLASS(SplFileInfo);
  SPL_ADD_CLASS(SplFileObject);
  SPL_ADD_CLASS(SplFixedArray);
  SPL_ADD_CLASS(SplHeap);
  SPL_ADD_CLASS(SplMinHeap);
  SPL_ADD_CLASS(SplMaxHeap);
  SPL_ADD_CLASS(SplObjectStorage);
  SPL_ADD_CLASS(SplObserver);
  SPL_ADD_CLASS(SplPriorityQueue);
  SPL_ADD_CLASS(SplQueue);
  SPL_ADD_CLASS(SplStack);
  SPL_ADD_CLASS(SplSubject);
  SPL_ADD_CLASS(SplTempFileObject);
  SPL_ADD_CLASS(UnderflowException);
  SPL_ADD_CLASS(UnexpectedValueException);
  return ret;
}

void throw_spl_exception(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string msg;
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  throw (Object)p_Exception(NEWOBJ(c_Exception)())->create(Variant(msg));
}

static bool s_inited = false;
static int64 s_hash_mask_handle = 0;
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
  snprintf(buf, sizeof(buf), "%032llx", s_hash_mask_handle ^ (int64)obj.get());
  return String(buf, CopyString);
}

Variant f_hphp_get_this() {
  return FrameInjection::GetThis();
}

Variant f_class_implements(CVarRef obj, bool autoload /* = true */) {
  String clsname;
  if (obj.isString()) {
    clsname = obj.toString();
  } else if (obj.isObject()) {
    clsname = obj.toObject()->o_getClassName();
  } else {
    return false;
  }

  const ClassInfo *info = ClassInfo::FindClass(clsname);
  if (info == NULL) {
    return false;
  }

  Array ret(Array::Create());
  ClassInfo::InterfaceVec ifs;
  info->getAllInterfacesVec(ifs);
  for (unsigned int i = 0; i < ifs.size(); i++) {
    ret.set(ifs[i], ifs[i]);
  }

  return ret;
}

Variant f_class_parents(CVarRef obj, bool autoload /* = true */) {
  String clsname;
  if (obj.isString()) {
    clsname = obj.toString();
  } else if (obj.isObject()) {
    clsname = obj.toObject()->o_getClassName();
  } else {
    return false;
  }

  const ClassInfo *info = ClassInfo::FindClass(clsname);
  if (info == NULL) {
    return false;
  }

  Array ret(Array::Create());
  ClassInfo::ClassVec parents;
  info->getAllParentsVec(parents);
  for (unsigned int i = 0; i < parents.size(); i++) {
    ret.set(parents[i], parents[i]);
  }

  return ret;
}

Variant f_iterator_apply(CVarRef obj, CVarRef func,
                         CArrRef params /* = null_array */) {
  if (!obj.instanceof("Traversable")) {
    return false;
  }
  Object pobj = obj.toObject();
  pobj->o_invoke("rewind", null_array, -1);
  int64 count = 0;
  while (same(pobj->o_invoke("valid", null_array, -1), true)) {
    if (!same(f_call_user_func_array(func, params), true)) {
      break;
    }
    ++count;
    pobj->o_invoke("next", null_array, -1);
  }
  return count;
}

Variant f_iterator_count(CVarRef obj) {
  if (!obj.instanceof("Traversable")) {
    return false;
  }
  Object pobj = obj.toObject();
  pobj->o_invoke("rewind", null_array, -1);
  int64 count = 0;
  while (same(pobj->o_invoke("valid", null_array, -1), true)) {
    ++count;
    pobj->o_invoke("next", null_array, -1);
  }
  return count;
}

Variant f_iterator_to_array(CVarRef obj, bool use_keys /* = true */) {
  if (!obj.instanceof("Traversable")) {
    return false;
  }
  Array ret(Array::Create());
  Object pobj = obj.toObject();
  pobj->o_invoke("rewind", null_array, -1);
  while (same(pobj->o_invoke("valid", null_array, -1), true)) {
    Variant val = pobj->o_invoke("current", null_array, -1);
    if (use_keys) {
      Variant key = pobj->o_invoke("key", null_array, -1);
      ret.set(key, val);
    } else {
      ret.append(val);
    }
    pobj->o_invoke("next", null_array, -1);
  }
  return ret;
}


bool f_spl_autoload_register(CVarRef autoload_function /* = null_variant */,
                             bool throws /* = true */,
                             bool prepend /* = false */) {
  if (autoload_function.same(s_spl_autoload_call)) {
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
  if (autoload_function.same(s_spl_autoload_call)) {
    AutoloadHandler::s_instance->removeAllHandlers();
  } else {
    AutoloadHandler::s_instance->removeHandler(autoload_function);
  }
  return true;
}

Variant f_spl_autoload_functions() {
  CArrRef handlers = AutoloadHandler::s_instance->getHandlers();
  if (handlers.empty())
    return false;
  else
    return handlers.values();
}

void f_spl_autoload_call(CStrRef class_name) {
  AutoloadHandler::s_instance->invokeHandler(class_name, true);
}

namespace {
class ExtensionList : public RequestEventHandler {
public:
  virtual void requestInit() {
    extensions = CREATE_VECTOR2(String(".inc"), String(".php"));
  }
  virtual void requestShutdown() {
    extensions.reset();
  }

  Array extensions;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(ExtensionList, s_extension_list);
}

String f_spl_autoload_extensions(CStrRef file_extensions /* = null_string */) {
  if (!file_extensions.isNull()) {
    s_extension_list->extensions = StringUtil::Explode(file_extensions, ",")
                                   .toArray();
    return file_extensions;
  }
  return StringUtil::Implode(s_extension_list->extensions, ",");
}

void f_spl_autoload(CStrRef class_name,
                    CStrRef file_extensions /* = null_string */) {
  Array ext = file_extensions.isNull()
              ? s_extension_list->extensions
              : StringUtil::Explode(file_extensions, ",").toArray();
  String lClass = StringUtil::ToLower(class_name);
  Globals *g = get_globals();
  bool found = false;
  for (ArrayIter iter(ext); iter; ++iter) {
    String fileName = lClass + iter.second();
    include(fileName, true, g, "", false);
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
}
