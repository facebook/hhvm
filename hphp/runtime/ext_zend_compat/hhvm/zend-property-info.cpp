/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext_zend_compat/hhvm/zend-property-info.h"

namespace HPHP {

template <class T>
void prop_to_zpi(zend_class_entry* ce,
                 const T* prop,
                 zend_property_info* info) {
  static_assert(std::is_same<T, HPHP::Class::Prop>::value ||
                std::is_same<T, HPHP::Class::SProp>::value, "");

  info->flags = 0;

  #define ATTR_TO_FLAGS                   \
    AT(AttrStatic, ZEND_ACC_STATIC)       \
    AT(AttrFinal, ZEND_ACC_FINAL)         \
    AT(AttrPublic, ZEND_ACC_PUBLIC)       \
    AT(AttrProtected, ZEND_ACC_PROTECTED) \
    AT(AttrPrivate, ZEND_ACC_PRIVATE)     \

  #define AT(ATTR, FLAG)              \
    if (prop->m_attrs & HPHP::ATTR) { \
      info->flags |= (FLAG);          \
    }
  ATTR_TO_FLAGS
  #undef AT

  info->name = prop->m_name->data();
  info->name_length = prop->m_name->size();
  info->doc_comment = prop->m_docComment->data();
  info->doc_comment_len = prop->m_docComment->size();
  info->ce = ce;
}

}
