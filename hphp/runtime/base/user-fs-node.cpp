/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/user-fs-node.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/std/ext_std_function.h"

#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/method-lookup.h"

namespace HPHP {

StaticString s_call("__call");
StaticString s_context("context");

UserFSNode::UserFSNode(Class* cls,
                       const req::ptr<StreamContext>& context /*= nullptr */) {
  VMRegAnchor _;
  const Func* ctor;
  m_cls = cls;
  if (LookupResult::MethodFoundWithThis !=
      lookupCtorMethod(ctor, m_cls, arGetContextClass(vmfp()))) {
    raise_error("Unable to call %s'n constructor", m_cls->name()->data());
  }

  m_obj = Object{m_cls};
  if (context.get()) {
    m_obj->setProp(nullptr, s_context.get(),
                   make_tv<KindOfResource>(context.get()->hdr()));
  } else {
    m_obj->setProp(nullptr, s_context.get(), make_tv<KindOfNull>());
  }
  auto ret = Variant::attach(
    g_context->invokeFuncFew(ctor, m_obj.get())
  );
  m_Call = lookupMethod(s_call.get());
}

Variant UserFSNode::invoke(const Func* func, const String& name,
                           const Array& args, bool& invoked) {
  VMRegAnchor _;

  // Assume failure
  invoked = false;

  // Public method, no private ancestor, no need for further checks (common)
  if (func &&
      !(func->attrs() & (AttrPrivate|AttrProtected|AttrAbstract)) &&
      !func->hasPrivateAncestor()) {
    auto ret = Variant::attach(
      g_context->invokeFunc(func, args, m_obj.get())
    );
    invoked = true;
    return ret;
  }

  // No explicitly defined function, no __call() magic method
  // Give up.
  if (!func && !m_Call) {
    return uninit_null();
  }

  CallerFrame cf;
  Class* ctx = arGetContextClass(cf());
  switch(lookupObjMethod(func, m_cls, name.get(), ctx)) {
    case LookupResult::MethodFoundWithThis:
    {
      auto ret = Variant::attach(
        g_context->invokeFunc(func, args, m_obj.get())
      );
      invoked = true;
      return ret;
    }

    case LookupResult::MagicCallFound:
    {
      auto ret = Variant::attach(
        g_context->invokeFunc(func, make_packed_array(name, args), m_obj.get())
      );
      invoked = true;
      return ret;
    }

    case LookupResult::MethodNotFound:
      // There's a method somewhere in the hierarchy, but none
      // which are accessible.
      /* fallthrough */
    case LookupResult::MagicCallStaticFound:
      // We're not calling statically, so this result is unhelpful
      // Also, it's never produced by lookupObjMethod, so it'll
      // never happen, but we must handle all enums
      return uninit_null();

    case LookupResult::MethodFoundNoThis:
      // Should never happen (Attr::Static check in ctor)
      assert(false);
      raise_error("%s::%s() must not be declared static",
                  m_cls->name()->data(), name.data());
      return uninit_null();
  }

  not_reached();
  return uninit_null();
}

const Func* UserFSNode::lookupMethod(const StringData* name) {
  const Func* f = m_cls->lookupMethod(name);
  if (!f) return nullptr;

  if (f->attrs() & AttrStatic) {
    raise_error("%s::%s() must not be declared static",
                m_cls->name()->data(), name->data());
  }
  return f;
}

}
