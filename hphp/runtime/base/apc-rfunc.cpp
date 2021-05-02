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

#include "hphp/runtime/base/apc-rfunc.h"
#include "hphp/runtime/base/apc-typed-value.h"

namespace HPHP {

APCRFunc::APCRFunc(RFuncData* rfuncData, APCHandle* generics):
  m_handle(APCKind::RFunc),
  m_entity(rfuncData->m_func->getNamedEntity()),
  m_name(rfuncData->m_func->name()),
  m_generics(generics)
{

}

APCHandle::Pair APCRFunc::Construct(RFuncData* rfuncData) {
  auto const generics = VarNR{rfuncData->m_arr};
  auto const handle = APCHandle::Create(const_variant_ref{generics},
                                        APCHandleLevel::Inner,
                                        true);

  auto const apcRFunc = new APCRFunc(rfuncData, handle.handle);
  return {apcRFunc->getHandle(), sizeof(APCRFunc) + handle.size};
}

Variant APCRFunc::Make(const APCHandle* handle) {
    auto apcrfunc = APCRFunc::fromHandle(handle);
    auto const f = Func::load(apcrfunc->m_entity, apcrfunc->m_name);
    auto const generics_arr = apcrfunc->m_generics->toLocal();
    auto const generics = generics_arr.getArrayData();
    generics->incRefCount();
    return Variant{RFuncData::newInstance(f, generics)};
}

void APCRFunc::Delete(APCHandle* handle) {
  auto const apcrfunc = fromHandle(handle);
  apcrfunc->m_generics->unreferenceRoot();
  delete apcrfunc;
}

const APCRFunc* APCRFunc::fromHandle(const APCHandle* handle) {
  assert(handle->checkInvariants() && handle->kind() == APCKind::RFunc);
  static_assert(offsetof(APCRFunc, m_handle) == 0, "");
  return reinterpret_cast<const APCRFunc*>(handle);
}

} // namespace HPHP
