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
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-request-local.h"
#include "zend_API.h"
#include "php.h"
#include "php_streams.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

ZEND_API int le_index_ptr;

namespace HPHP {
  struct ZendResourceWrapper;

  // When the request ends, destroy all remaining Zend resource wrappers by
  // freeing them. Their destructors will automatically call the appropriate
  // Zend destructor.
  struct ZendResourceWrapperDestroyer {
    void operator()(ZendResourceWrapper* wrapper) {
      req::destroy_raw(wrapper);
    }
  };
}

ZEND_REQUEST_LOCAL_VECTOR(HPHP::ZendResourceWrapper*,
                          HPHP::ZendResourceWrapperDestroyer,
                          tl_regular_list);
typedef HPHP::ZendRequestLocalVector<HPHP::ZendResourceWrapper*,
                                     HPHP::ZendResourceWrapperDestroyer
                                     >::container zend_rsrc_list;

namespace {
  zend_rsrc_list& RL() {
    return tl_regular_list.get()->get();
  }

  HPHP::ZendResourceWrapper* zend_list_id_to_wrapper(int id) {
    if (id >= 0 && id < RL().size()) {
      return RL()[id];
    } else {
      return nullptr;
    }
  }

}

namespace HPHP {
  static std::vector<zend_rsrc_list_dtors_entry> s_resource_dtors;

  // Represents the actual Zend resource. ZendResourceData contains the index
  // into the Zend resource table where these are stored. If the table entry is
  // null, the resource has been closed. In a few circumstances, a ResourceData
  // instance can be stored instead of a Zend resource.
  struct ZendResourceWrapper {
    ZendResourceWrapper(void* ptr, int type, int id)
      : raw_ptr{ptr},
        id{id},
        type{type},
        refcount{1},
        raw_ptr_is_resource{false} {}
    ZendResourceWrapper(ResourceData* ptr, int id)
      : raw_ptr{ptr},
        id{id},
        type{php_file_le_stream()},
        refcount{1},
        raw_ptr_is_resource{true} { ptr->incRefCount(); }
    ~ZendResourceWrapper();

    void* raw_ptr;
    int id;
    int type;
    int refcount;
    bool raw_ptr_is_resource; // Whether we're storing a ResourceData or a Zend
                              // resource.
  };

  ZendResourceWrapper::~ZendResourceWrapper() {
    if (!raw_ptr_is_resource) {
      // If we're storing a (valid) Zend resource, call its destructor (if any).
      if (type >= 0) {
        auto& dtor = s_resource_dtors.at(type);
        if (dtor.list_dtor_ex) {
          // Zend resource destructors receive a ZendResourceData as their sole
          // parameter. They use this to determine the pointer to the Zend
          // resource (the pointer is normally null). Problem is, we don't have
          // a ZendResourceData handy, as we should only get here if they've all
          // been released. So, allocate a temporary one (containing the Zend
          // resource pointer) and pass it into the destructor.
          auto temp = HPHP::req::make<zend_rsrc_list_entry>(raw_ptr, type);
          temp->id = id;

          TSRMLS_FETCH();
          VMRegAnchor _;
          dtor.list_dtor_ex(temp.get() TSRMLS_CC);
        }
      }
    } else {
      // Otherwise we're storing a ResourceData instance, so just release our
      // reference to it.
      decRefRes(static_cast<ResourceData*>(raw_ptr));
    }
  }

  ZendResourceData::~ZendResourceData() {
    // Last reference to the Zend resource is gone, so destroy the resource if
    // it hasn't already been destroyed.
    if (auto wrapper = zend_list_id_to_wrapper(id)) {
      if (--wrapper->refcount <= 0) {
        // Important: null out the pointer in the resource table before
        // destroying the resource. This prevents recursion when we allocate a
        // temporary ZendResourceData before calling the Zend destructor.
        RL()[id] = nullptr;
        HPHP::req::destroy_raw(wrapper);
      }
    }
  }

  const String& ZendResourceData::o_getClassNameHook() const {
    auto& dtor = s_resource_dtors.at(type);

    if (dtor.type_name_str.empty()) {
      dtor.type_name_str = makeStaticString(dtor.type_name);
    }

    return dtor.type_name_str;
  }

  bool ZendResourceData::isInvalid() const {
    return !zend_list_id_to_wrapper(id);
  }

}

///////////////////////////////////////////////////////////////////////////////////

ZEND_API int zend_list_insert(void *ptr, int type TSRMLS_DC) {
  int id = RL().size();
  auto wrapper =
    HPHP::req::make_raw<HPHP::ZendResourceWrapper>(ptr, type, id);
  RL().push_back(wrapper);
  return id;
}

ZEND_API int _zend_list_delete(int id TSRMLS_DC) {
  auto wrapper = zend_list_id_to_wrapper(id);
  if (wrapper) {
    if (--wrapper->refcount <= 0) {
      RL()[id] = nullptr;
      HPHP::req::destroy_raw(wrapper);
    }
    return SUCCESS;
  } else {
    return FAILURE;
  }
}

ZEND_API void *_zend_list_find(int id, int *type TSRMLS_DC) {
  auto* wrapper = zend_list_id_to_wrapper(id);
  if (wrapper) {
    *type = wrapper->type;
    return wrapper->raw_ptr;
  } else {
    *type = -1;
    return nullptr;
  }
}

ZEND_API int _zend_list_addref(int id TSRMLS_DC) {
  auto wrapper = zend_list_id_to_wrapper(id);
  if (wrapper) {
    ++wrapper->refcount;
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
  lde.resource_id = HPHP::s_resource_dtors.size();
  lde.type = ZEND_RESOURCE_LIST_TYPE_EX;
  lde.type_name = type_name;

  HPHP::s_resource_dtors.push_back(lde);
  return lde.resource_id;
}

int zval_get_resource_id(const zval &z) {
  zend_rsrc_list_entry* le =
    dynamic_cast<zend_rsrc_list_entry*>(z.tv()->m_data.pres);
  if (le) {
    return le->id;
  }

  int id = RL().size();
  auto wrapper =
    HPHP::req::make_raw<HPHP::ZendResourceWrapper>(z.tv()->m_data.pres, id);
  RL().push_back(wrapper);
  return id;
}

HPHP::ResourceData *zend_list_id_to_resource_data(int id TSRMLS_DC) {
  auto wrapper = zend_list_id_to_wrapper(id);
  if (wrapper) {
    auto resource = HPHP::newres<zend_rsrc_list_entry>(nullptr, wrapper->type);
    resource->id = wrapper->id;
    return resource;
  } else {
    return nullptr;
  }
}
