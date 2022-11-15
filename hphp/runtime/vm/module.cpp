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

#include "hphp/runtime/vm/module.h"

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/rds-symbol.h"
#include "hphp/runtime/base/rds-util.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/string-data.h"

#include "hphp/runtime/vm/func.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

//////////////////////////////////////////////////////////////////////

void Module::prettyPrint(std::ostream& out) const {
  out << "Module " << name->data();
  if (attrs & AttrUnique)     out << " (unique)";
  if (attrs & AttrPersistent) out << " (persistent)";
  out << std::endl;
}

Module* Module::lookup(const StringData* name) {
  assertx(name->isStatic());
  if (RO::RepoAuthoritative) {
    auto const link = rds::attachModuleCache<rds::Mode::Persistent>(name);
    return link.bound() ? *link : nullptr;
  }
  auto const link = rds::attachModuleCache<rds::Mode::Normal>(name);
  return link.bound() ? *link : nullptr;
}

Module* Module::load(const StringData* name) {
  Module* m = Module::lookup(name);
  if (LIKELY(m != nullptr)) return m;
  if (AutoloadHandler::s_instance->autoloadModule(
        const_cast<StringData*>(name))) {
    m = Module::lookup(name);
  }
  return m;
}

void Module::def(Module* m) {
  assertx(m->name->isStatic());
  if (RO::RepoAuthoritative) {
    auto const link = rds::bindModuleCache<rds::Mode::Persistent>(m->name);
    assertx(m->attrs & AttrPersistent);
    always_assert(*link == nullptr);
    link.initWith(m);
    return;
  }
  auto const link = rds::bindModuleCache<rds::Mode::Normal>(m->name);
  if (link.isInit()) {
    raise_error("Module already defined: %s", m->name->data());
  }
  link.initWith(m);
}

template <typename Sym, typename Ctx>
bool will_symbol_raise_module_boundary_violation(const Sym* symbol,
                                                 const Ctx* context) {
  assertx(symbol && context);
  return symbol->isInternal() &&
         context->moduleName() != symbol->moduleName();
}

template bool will_symbol_raise_module_boundary_violation(const Func*, const Func*);
template bool will_symbol_raise_module_boundary_violation(const Func*, const MemberLookupContext*);
template bool will_symbol_raise_module_boundary_violation(const Class::Prop*, const MemberLookupContext*);
template bool will_symbol_raise_module_boundary_violation(const Class::Prop*, const Func*);
template bool will_symbol_raise_module_boundary_violation(const Class*, const Func*);
template bool will_symbol_raise_module_boundary_violation(const Class::SProp*, const MemberLookupContext*);
template bool will_symbol_raise_module_boundary_violation(const Class::SProp*, const Func*);


bool Module::warningsEnabled(const Func* f) {
  return f->attrs() & AttrInternalSoft;
}

} // namespace HPHP
