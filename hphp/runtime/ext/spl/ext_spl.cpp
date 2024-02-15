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
  s_rewind("rewind"),
  s_valid("valid"),
  s_next("next"),
  s_current("current"),
  s_key("key"),
  s_getIterator("getIterator");

void throw_spl_exception(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
  ATTRIBUTE_PRINTF(1,2);
void throw_spl_exception(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string msg;
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  SystemLib::throwExceptionObject(Variant(msg));
}

static uint64_t s_hash_mask_handle = 0;
static std::once_flag s_hash_mask_handle_inited;

String HHVM_FUNCTION(spl_object_hash, const Object& obj) {
  std::call_once(s_hash_mask_handle_inited, [] {
    HHVM_FN(mt_srand)();

    uint64_t mask = 0;
    mask ^= HHVM_FN(mt_rand)(); mask <<= 16;
    mask ^= HHVM_FN(mt_rand)(); mask <<= 16;
    mask ^= HHVM_FN(mt_rand)(); mask <<= 16;
    mask ^= HHVM_FN(mt_rand)();

    s_hash_mask_handle = mask;
  });

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
  return Variant{g_context->getThis()};
}

Variant HHVM_FUNCTION(class_implements, const Variant& obj,
                                        bool autoload /* = true */) {
  Class* cls;
  if (obj.isString() || obj.isLazyClass()) {
    auto const name = obj.isString() ? obj.getStringData() :
                                       obj.toLazyClassVal().name();
    cls = Class::get(name, autoload);
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
  } else if (obj.isClass()) {
    cls = obj.toClassVal();
  } else {
    raise_warning("class_implements(): object or string expected");
    return false;
  }
  Array ret(Array::CreateDict());
  const Class::InterfaceMap& ifaces = cls->allInterfaces();
  for (int i = 0, size = ifaces.size(); i < size; i++) {
    ret.set(ifaces[i]->nameStr(),
            make_tv<KindOfPersistentString>(ifaces[i]->name()));
  }
  return ret;
}

Variant HHVM_FUNCTION(class_parents, const Variant& obj,
                                     bool autoload /* = true */) {
  Class* cls;
  if (obj.isString() || obj.isLazyClass()) {
    auto const name = obj.isString() ? obj.getStringData() :
                                       obj.toLazyClassVal().name();
    cls = Class::get(name, autoload);
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
  } else if (obj.isClass()) {
    cls = obj.toClassVal();
  } else {
    raise_warning("class_parents(): object or string expected");
    return false;
  }
  Array ret(Array::CreateDict());
  for (cls = cls->parent(); cls; cls = cls->parent()) {
    ret.set(cls->nameStr(), make_tv<KindOfPersistentString>(cls->name()));
  }
  return ret;
}

Variant HHVM_FUNCTION(class_uses, const Variant& obj,
                                  bool autoload /* = true */) {
  Class* cls;
  if (obj.isString() || obj.isLazyClass()) {
    auto const name = obj.isString() ? obj.getStringData() :
                                       obj.toLazyClassVal().name();
    cls = Class::get(name, autoload);
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
  } else if (obj.isClass()) {
    cls = obj.toClassVal();
  } else {
    raise_warning("class_uses(): object or string expected");
    return false;
  }
  auto &usedTraits = cls->preClass()->usedTraits();
  DictInit ret(usedTraits.size());
  for (auto const& traitName : usedTraits) {
    ret.set(StrNR(traitName), VarNR(traitName).tv());
  }
  return ret.toArray();
}

struct ExtensionList final : RequestEventHandler {
  void requestInit() override {
    extensions = make_vec_array(String(".inc"), String(".php"));
  }
  void requestShutdown() override {
    extensions.reset();
  }

  Array extensions;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(ExtensionList, s_extension_list);

///////////////////////////////////////////////////////////////////////////////

struct SPLExtension final : Extension {
  SPLExtension() : Extension("spl", "0.2", NO_ONCALL_YET) { }
  void moduleRegisterNative() override {
    HHVM_FE(spl_object_hash);
    HHVM_FE(hphp_object_pointer);
    HHVM_FE(hphp_get_this);
    HHVM_FE(class_implements);
    HHVM_FE(class_parents);
    HHVM_FE(class_uses);
  }
} s_SPL_extension;

///////////////////////////////////////////////////////////////////////////////
}
