/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/spl/ext_spl.h"

#include "hphp/runtime/ext/std/ext_std_classobj.h"
#include "hphp/runtime/ext/std/ext_std_math.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/string/ext_string.h"

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/base/glob-stream-wrapper.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/string-util.h"

#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/system/systemlib.h"
#include "hphp/util/string-vsnprintf.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_spl_autoload("spl_autoload"),
  s_spl_autoload_call("spl_autoload_call"),
  s_rewind("rewind"),
  s_valid("valid"),
  s_next("next"),
  s_current("current"),
  s_key("key"),
  s_getIterator("getIterator"),
  s_directory_iterator("DirectoryIterator");


void throw_spl_exception(const char *fmt, ...) ATTRIBUTE_PRINTF(1,2);
void throw_spl_exception(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string msg;
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  SystemLib::throwExceptionObject(Variant(msg));
}

static bool s_inited = false;
static int64_t s_hash_mask_handle = 0;
static Mutex s_mutex;

String HHVM_FUNCTION(spl_object_hash, const Object& obj) {
  if (!s_inited) {
    Lock lock(s_mutex);
    if (!s_inited) {
      HHVM_FN(mt_srand)();
      s_hash_mask_handle |= HHVM_FN(mt_rand)(); s_hash_mask_handle <<= 16;
      s_hash_mask_handle |= HHVM_FN(mt_rand)(); s_hash_mask_handle <<= 16;
      s_hash_mask_handle |= HHVM_FN(mt_rand)(); s_hash_mask_handle <<= 16;
      s_hash_mask_handle |= HHVM_FN(mt_rand)();
      s_inited = true;
    }
  }

  char buf[33];
  // Using the object address here would interfere with a moving GC algorithm.
  // See t6299529.
  snprintf(buf, sizeof(buf), "%032" PRIx64,
           s_hash_mask_handle ^ (int64_t)obj.get());
  return String(buf, CopyString);
}

// Using the object address here could interfere with a moving GC algorithm.
// See t6299529.
int64_t HHVM_FUNCTION(hphp_object_pointer, const Object& obj) {
  return (int64_t)obj.get();
}

Variant HHVM_FUNCTION(hphp_get_this) {
  return g_context->getThis();
}

Variant HHVM_FUNCTION(class_implements, const Variant& obj,
                                        bool autoload /* = true */) {
  Class* cls;
  if (obj.isString()) {
    cls = Unit::getClass(obj.getStringData(), autoload);
    if (!cls) {
      String err = "class_implements(): Class %s does not exist";
      if (autoload) {
        err += " and could not be loaded";
      }
      raise_warning(err.c_str(), obj.toString().c_str());
      return false;
    }
  } else if (obj.isObject()) {
    cls = obj.getObjectData()->getVMClass();
  } else {
    raise_warning("class_implements(): object or string expected");
    return false;
  }
  Array ret(Array::Create());
  const Class::InterfaceMap& ifaces = cls->allInterfaces();
  for (int i = 0, size = ifaces.size(); i < size; i++) {
    ret.set(ifaces[i]->nameStr(), VarNR(ifaces[i]->name()));
  }
  return ret;
}

Variant HHVM_FUNCTION(class_parents, const Variant& obj,
                                     bool autoload /* = true */) {
  Class* cls;
  if (obj.isString()) {
    cls = Unit::getClass(obj.getStringData(), autoload);
    if (!cls) {
      String err = "class_parents(): Class %s does not exist";
      if (autoload) {
        err += " and could not be loaded";
      }
      raise_warning(err.c_str(), obj.toString().c_str());
      return false;
    }
  } else if (obj.isObject()) {
    cls = obj.getObjectData()->getVMClass();
  } else {
    raise_warning("class_parents(): object or string expected");
    return false;
  }
  Array ret(Array::Create());
  for (cls = cls->parent(); cls; cls = cls->parent()) {
    ret.set(cls->nameStr(), VarNR(cls->name()));
  }
  return ret;
}

Variant HHVM_FUNCTION(class_uses, const Variant& obj,
                                  bool autoload /* = true */) {
  Class* cls;
  if (obj.isString()) {
    cls = Unit::getClass(obj.getStringData(), autoload);
    if (!cls) {
      String err = "class_uses(): Class %s does not exist";
      if (autoload) {
        err += " and could not be loaded";
      }
      raise_warning(err.c_str(), obj.toString().c_str());
      return false;
    }
  } else if (obj.isObject()) {
    cls = obj.getObjectData()->getVMClass();
  } else {
    raise_warning("class_uses(): object or string expected");
    return false;
  }
  auto &usedTraits = cls->preClass()->usedTraits();
  ArrayInit ret(usedTraits.size(), ArrayInit::Map{});
  for (auto const& traitName : usedTraits) {
    ret.set(StrNR(traitName), VarNR(traitName));
  }
  return ret.toArray();
}

#define CHECK_TRAVERSABLE_IMPL(obj, ret) \
  if (!obj.isObject() || \
      !obj.getObjectData()->instanceof(SystemLib::s_TraversableClass)) { \
    raise_recoverable_error("Argument must implement interface Traversable"); \
    return ret; \
  }

Object get_traversable_object_iterator(const Variant& obj) {
  bool isIteratorAggregate;
  Object itObj = obj.getObjectData()
    ->iterableObject(isIteratorAggregate, true);

  if (!isIteratorAggregate) {
    if (!obj.getObjectData()->instanceof(
        SystemLib::s_IteratorAggregateClass)) {
      raise_error("Objects returned by getIterator() must be traversable or "
                  "implement interface Iterator");
    } else {
      raise_error(
        "Class %s must implement interface Traversable as part of either "
        "Iterator or IteratorAggregate",
        obj.toObject()->getClassName().data()
      );
    }
  }

  return itObj;
}


Variant HHVM_FUNCTION(iterator_apply, const Variant& obj, const Variant& func,
                         const Array& params /* = null_array */) {
  VMRegAnchor _;
  CHECK_TRAVERSABLE_IMPL(obj, 0);
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

Variant HHVM_FUNCTION(iterator_count, const Variant& obj) {
  VMRegAnchor _;
  CHECK_TRAVERSABLE_IMPL(obj, 0);
  Object pobj = get_traversable_object_iterator(obj);
  pobj->o_invoke_few_args(s_rewind, 0);
  int64_t count = 0;
  while (same(pobj->o_invoke_few_args(s_valid, 0), true)) {
    ++count;
    pobj->o_invoke_few_args(s_next, 0);
  }
  return count;
}

Array HHVM_FUNCTION(iterator_to_array, const Variant& obj,
                                         bool use_keys /* = true */) {
  VMRegAnchor _;
  Array ret(Array::Create());
  CHECK_TRAVERSABLE_IMPL(obj, ret);
  Object pobj = get_traversable_object_iterator(obj);
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

bool HHVM_FUNCTION(spl_autoload_register,
                   const Variant& autoload_function /* = null_variant */,
                   bool throws /* = true */,
                   bool prepend /* = false */) {
  if (same(autoload_function, s_spl_autoload_call)) {
    if (throws) {
      throw_spl_exception("Function spl_autoload_call()"
                      "cannot be registered");
    }
    return false;
  }
  const Variant& func = autoload_function.isNull() ?
                 s_spl_autoload : autoload_function;
  bool res = AutoloadHandler::s_instance->addHandler(func, prepend);
  if (!res && throws) {
    throw_spl_exception("Invalid autoload_function specified");
  }
  return res;
}

bool HHVM_FUNCTION(spl_autoload_unregister, const Variant& autoload_function) {
  if (same(autoload_function, s_spl_autoload_call)) {
    AutoloadHandler::s_instance->removeAllHandlers();
  } else {
    AutoloadHandler::s_instance->removeHandler(autoload_function);
  }
  return true;
}

Variant HHVM_FUNCTION(spl_autoload_functions) {
  const Array& handlers = AutoloadHandler::s_instance->getHandlers();
  if (handlers.isNull()) {
    return false;
  } else {
    return handlers.values();
  }
}

void HHVM_FUNCTION(spl_autoload_call, const String& class_name) {
  AutoloadHandler::s_instance->autoloadClass(class_name, true);
}

namespace {
struct ExtensionList final : RequestEventHandler {
  void requestInit() override {
    extensions = make_packed_array(String(".inc"), String(".php"));
  }
  void requestShutdown() override {
    extensions.reset();
  }

  Array extensions;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(ExtensionList, s_extension_list);
}

String HHVM_FUNCTION(spl_autoload_extensions,
                     const String& file_extensions /* = null_string */) {
  if (!file_extensions.empty()) {
    s_extension_list->extensions = StringUtil::Explode(file_extensions, ",")
                                   .toArray();
    return file_extensions;
  }
  return StringUtil::Implode(s_extension_list->extensions, ",");
}

///////////////////////////////////////////////////////////////////////////////

template <class T>
static req::ptr<T> getDir(const Object& dir_iter) {
  static_assert(std::is_base_of<Directory, T>::value,
                "Only cast to directories");
  return cast<T>(*dir_iter->o_realProp("dir", 0, s_directory_iterator));
}

static Variant HHVM_METHOD(DirectoryIterator, hh_readdir) {
  auto dir = getDir<Directory>(ObjNR(this_).asObject());

  if (auto array_dir = dyn_cast<ArrayDirectory>(dir)) {
    auto prop = this_->o_realProp("dirName", 0, s_directory_iterator);
    *prop = array_dir->path();
  }

  return HHVM_FN(readdir)(Resource(dir));
}

static int64_t HHVM_METHOD(GlobIterator, count) {
  return getDir<ArrayDirectory>(ObjNR(this_).asObject())->size();
}

///////////////////////////////////////////////////////////////////////////////

class SPLExtension final : public Extension {
public:
  SPLExtension() : Extension("spl", "0.2") { }
  void moduleLoad(const IniSetting::Map& ini, Hdf config) override {
    HHVM_ME(DirectoryIterator, hh_readdir);
    HHVM_ME(GlobIterator, count);
  }
  void moduleInit() override {
    HHVM_FE(spl_object_hash);
    HHVM_FE(hphp_object_pointer);
    HHVM_FE(hphp_get_this);
    HHVM_FE(class_implements);
    HHVM_FE(class_parents);
    HHVM_FE(class_uses);
    HHVM_FE(iterator_apply);
    HHVM_FE(iterator_count);
    HHVM_FE(iterator_to_array);
    HHVM_FE(spl_autoload_call);
    HHVM_FE(spl_autoload_extensions);
    HHVM_FE(spl_autoload_functions);
    HHVM_FE(spl_autoload_register);
    HHVM_FE(spl_autoload_unregister);

    loadSystemlib();
  }
} s_SPL_extension;

///////////////////////////////////////////////////////////////////////////////
}
