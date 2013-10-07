/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2013 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

/* resource lists */

#include "zend.h"
#include "zend_list.h"
// has to be before zend_API since that defines getThis()
#include "hphp/runtime/ext_zend_compat/hhvm/ZendRequestLocal.h"
#include "zend_API.h"
#include "php.h"
#include "php_streams.h"

ZEND_API int le_index_ptr;

namespace HPHP {
  IMPLEMENT_OBJECT_ALLOCATION(ZendResourceData);

  ZEND_REQUEST_LOCAL_MAP(int, zend_rsrc_list_dtors_entry, s_resource_dtors);
  const String& ZendResourceData::o_getClassNameHook() const {
    auto& dtor = s_resource_dtors.get()->get()[type];

    if (dtor.type_name_str.empty()) {
      dtor.type_name_str = dtor.type_name;
    }

    return dtor.type_name_str;
  }
}

ZEND_REQUEST_LOCAL_LIST(zend_rsrc_list_entry*, s_regular_list);
typedef ZendRequestLocalList<zend_rsrc_list_entry*>::list zend_rsrc_list;

zend_rsrc_list& RL() {
  return s_regular_list.get()->get();
}

static zend_rsrc_list_entry *zend_list_id_to_entry(int id TSRMLS_DC) {
  if (id < RL().size()) {
    return RL().at(id);
  } else {
    return NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////////

ZEND_API int zend_list_insert(void *ptr, int type TSRMLS_DC) {
  zend_rsrc_list_entry* le = NEWOBJ(zend_rsrc_list_entry)(ptr, type);
  // TODO is this right?
  le->incRefCount();
  le->incRefCount();
  RL().push_back(le);
  int id = RL().size() - 1;
  le->id = id;
  return id;
}

ZEND_API int _zend_list_delete(int id TSRMLS_DC) {
  zend_rsrc_list_entry* le = zend_list_id_to_entry(id);
  if (le) {
    int refcount = le->getCount();
    decRefRes(le);
    if (refcount <= 1) {
      RL()[id] = nullptr;
      return SUCCESS;
    } else {
      return SUCCESS;
    }
  } else {
    return FAILURE;
  }
}

ZEND_API void *_zend_list_find(int id, int *type TSRMLS_DC) {
  zend_rsrc_list_entry* le = zend_list_id_to_entry(id);
  HPHP::ZendNormalResourceDataHolder* holder =
    dynamic_cast<HPHP::ZendNormalResourceDataHolder*>(le);
  if (holder) {
    *type = php_file_le_stream();
    return holder->getResourceData();
  } else if (le) {
    *type = le->type;
    return le->ptr;
  } else {
    *type = -1;
    return NULL;
  }
}

ZEND_API int _zend_list_addref(int id TSRMLS_DC) {
  zend_rsrc_list_entry* le = zend_list_id_to_entry(id);
  if (le) {
    le->incRefCount();
    return SUCCESS;
  } else {
    return FAILURE;
  }
}


ZEND_API int zend_register_resource(zval *rsrc_result, void *rsrc_pointer, int rsrc_type TSRMLS_DC)
{
  int rsrc_id;

  rsrc_id = zend_list_insert(rsrc_pointer, rsrc_type TSRMLS_CC);

  if (rsrc_result) {
    ZVAL_RESOURCE(rsrc_result, rsrc_id);
  }

  return rsrc_id;
}


ZEND_API void *zend_fetch_resource(zval **passed_id TSRMLS_DC, int default_id, const char *resource_type_name, int *found_resource_type, int num_resource_types, ...)
{
  int id;
  int actual_resource_type;
  void *resource;
  va_list resource_types;
  int i;
  const char *space;
  const char *class_name;

  if (default_id==-1) { /* use id */
    if (!passed_id) {
      if (resource_type_name) {
        class_name = get_active_class_name(&space TSRMLS_CC);
        zend_error(E_WARNING, "%s%s%s(): no %s resource supplied", class_name, space, get_active_function_name(TSRMLS_C), resource_type_name);
      }
      return NULL;
    } else if (Z_TYPE_PP(passed_id) != IS_RESOURCE) {
      if (resource_type_name) {
        class_name = get_active_class_name(&space TSRMLS_CC);
        zend_error(E_WARNING, "%s%s%s(): supplied argument is not a valid %s resource", class_name, space, get_active_function_name(TSRMLS_C), resource_type_name);
      }
      return NULL;
    }
    id = Z_RESVAL_PP(passed_id);
  } else {
    id = default_id;
  }

  resource = zend_list_find(id, &actual_resource_type);
  if (!resource) {
    if (resource_type_name) {
      class_name = get_active_class_name(&space TSRMLS_CC);
      zend_error(E_WARNING, "%s%s%s(): %d is not a valid %s resource", class_name, space, get_active_function_name(TSRMLS_C), id, resource_type_name);
    }
    return NULL;
  }

  va_start(resource_types, num_resource_types);
  for (i=0; i<num_resource_types; i++) {
    if (actual_resource_type == va_arg(resource_types, int)) {
      va_end(resource_types);
      if (found_resource_type) {
        *found_resource_type = actual_resource_type;
      }
      return resource;
    }
  }
  va_end(resource_types);

  if (resource_type_name) {
    class_name = get_active_class_name(&space TSRMLS_CC);
    zend_error(E_WARNING, "%s%s%s(): supplied resource is not a valid %s resource", class_name, space, get_active_function_name(TSRMLS_C), resource_type_name);
  }

  return NULL;
}

int zend_init_rsrc_list(TSRMLS_D) {
  return SUCCESS;
}

ZEND_API int zend_register_list_destructors_ex(rsrc_dtor_func_t ld, rsrc_dtor_func_t pld, const char *type_name, int module_number)
{
  zend_rsrc_list_dtors_entry lde;

#if 0
  printf("Registering destructors %d for module %d\n", list_destructors.nNextFreeElement, module_number);
#endif

  lde.list_dtor = NULL;
  lde.plist_dtor = NULL;
  lde.list_dtor_ex = ld;
  lde.plist_dtor_ex = pld;
  lde.module_number = module_number;
  lde.resource_id = HPHP::s_resource_dtors.get()->get().size();
  lde.type = ZEND_RESOURCE_LIST_TYPE_EX;
  lde.type_name = type_name;

  HPHP::s_resource_dtors.get()->get()[lde.resource_id] = lde;
  return lde.resource_id;
}

int zval_get_resource_id(const zval &z) {
  zend_rsrc_list_entry* le =
    dynamic_cast<zend_rsrc_list_entry*>(z.tv()->m_data.pres);
  if (le) {
    return le->id;
  }

  // Make a zend_rsrc_list_entry and return that
  le = NEWOBJ(HPHP::ZendNormalResourceDataHolder)(z.tv()->m_data.pres);
  RL().push_back(le);
  int id = RL().size() - 1;
  le->id = id;
  return id;
}

HPHP::ResourceData *zend_list_id_to_resource_data(int id TSRMLS_DC) {
  zend_rsrc_list_entry* le = zend_list_id_to_entry(id);
  HPHP::ZendNormalResourceDataHolder* holder =
    dynamic_cast<HPHP::ZendNormalResourceDataHolder*>(le);
  return holder ? holder->getResourceData() : le;
}
