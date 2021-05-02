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

#include "hphp/runtime/base/apc-rclass-meth.h"
#include "hphp/runtime/base/apc-typed-value.h"

namespace HPHP {

APCRClsMeth::APCRClsMeth(RClsMethData* data, APCHandle* generics):
  m_handle(APCKind::RClsMeth),
  m_cls_name(data->m_cls->name()),
  m_func_name(data->m_func->name()),
  m_generics(generics)
{}

APCHandle::Pair APCRClsMeth::Construct(RClsMethData* data) {
  auto const generics = VarNR{data->m_arr};
  auto const handle = APCHandle::Create(const_variant_ref{generics},
                                        APCHandleLevel::Inner,
                                        true);
  auto const apcRClsMeth = new APCRClsMeth(data, handle.handle);
  return {apcRClsMeth->getHandle(), sizeof(APCRClsMeth) + handle.size};
}

Variant APCRClsMeth::Make(const APCHandle* handle) {
  auto apc_rclsmeth = APCRClsMeth::fromHandle(handle);
  auto const cls = Class::load(apc_rclsmeth->m_cls_name);
  auto const func = cls->lookupMethod(apc_rclsmeth->m_func_name);
  auto const generics = apc_rclsmeth->m_generics->toLocal();
  auto const generics_arr = generics.getArrayData();
  generics_arr->incRefCount();
  return Variant{RClsMethData::create(cls, func, generics_arr)};
}

void APCRClsMeth::Delete(APCHandle* handle) {
  auto const apc_rclsmeth = fromHandle(handle);
  apc_rclsmeth->m_generics->unreferenceRoot();
  delete apc_rclsmeth;
}

const APCRClsMeth* APCRClsMeth::fromHandle(const APCHandle* handle) {
  assert(handle->checkInvariants() && handle->kind() == APCKind::RClsMeth);
  static_assert(offsetof(APCRClsMeth, m_handle) == 0, "");
  return reinterpret_cast<const APCRClsMeth*>(handle);
}

} // namespace HPHP
