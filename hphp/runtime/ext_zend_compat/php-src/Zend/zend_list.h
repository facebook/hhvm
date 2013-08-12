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

#ifndef ZEND_LIST_H
#define ZEND_LIST_H

#include "zend_hash.h"

BEGIN_EXTERN_C()

#define ZEND_RESOURCE_LIST_TYPE_STD  1
#define ZEND_RESOURCE_LIST_TYPE_EX  2

namespace HPHP {
  class ZendResourceData : public SweepableResourceData {
    public:
      virtual void sweep();
      ZendResourceData(void* ptr, int type) : ptr(ptr), type(type) {}
      ~ZendResourceData() {}
      void* ptr;
      int type;
      int id;
  };
  class ZendNormalResourceDataHolder : public ZendResourceData {
    public:
      ZendNormalResourceDataHolder(ResourceData* rd) :
          ZendResourceData(nullptr, -1), m_rd(rd) {}
      ~ZendNormalResourceDataHolder() {}
      ResourceData* getResourceData() { return m_rd; }
    private:
      ResourceData* m_rd;
  };
};
typedef HPHP::ZendResourceData zend_rsrc_list_entry;
typedef void (rsrc_dtor_func_t)(zend_rsrc_list_entry*);
ZEND_API int zend_register_list_destructors_ex(rsrc_dtor_func_t ld, rsrc_dtor_func_t pld, const char *type_name, int module_number);

int zend_init_rsrc_list(TSRMLS_D);
int zend_init_rsrc_plist(TSRMLS_D);
void zend_destroy_rsrc_list(HashTable *ht TSRMLS_DC);

ZEND_API int zend_list_insert(void *ptr, int type TSRMLS_DC);
ZEND_API int _zend_list_addref(int id TSRMLS_DC);
ZEND_API int _zend_list_delete(int id TSRMLS_DC);
ZEND_API void *_zend_list_find(int id, int *type TSRMLS_DC);

// For ZVAL_RESOURCE()
HPHP::ResourceData *zend_list_id_to_resource_data(int id TSRMLS_DC);

#define zend_list_addref(id)    _zend_list_addref(id TSRMLS_CC)
#define zend_list_delete(id)    _zend_list_delete(id TSRMLS_CC)
#define zend_list_find(id, type)  _zend_list_find(id, type TSRMLS_CC)

ZEND_API int zend_register_resource(zval *rsrc_result, void *rsrc_pointer, int rsrc_type TSRMLS_DC);
ZEND_API void *zend_fetch_resource(zval **passed_id TSRMLS_DC, int default_id, const char *resource_type_name, int *found_resource_type, int num_resource_types, ...);

extern ZEND_API int le_index_ptr;  /* list entry type for index pointers */

// for zval_get_resource_id()
int zval_get_resource_id(const zval &z);

#define ZEND_VERIFY_RESOURCE(rsrc)    \
  if (!rsrc) {            \
    RETURN_FALSE;          \
  }

#define ZEND_FETCH_RESOURCE(rsrc, rsrc_type, passed_id, default_id, resource_type_name, resource_type)  \
  rsrc = (rsrc_type) zend_fetch_resource(passed_id TSRMLS_CC, default_id, resource_type_name, NULL, 1, resource_type);  \
  ZEND_VERIFY_RESOURCE(rsrc);

#define ZEND_FETCH_RESOURCE_NO_RETURN(rsrc, rsrc_type, passed_id, default_id, resource_type_name, resource_type)  \
  (rsrc = (rsrc_type) zend_fetch_resource(passed_id TSRMLS_CC, default_id, resource_type_name, NULL, 1, resource_type))

#define ZEND_FETCH_RESOURCE2(rsrc, rsrc_type, passed_id, default_id, resource_type_name, resource_type1, resource_type2)  \
  rsrc = (rsrc_type) zend_fetch_resource(passed_id TSRMLS_CC, default_id, resource_type_name, NULL, 2, resource_type1, resource_type2);  \
  ZEND_VERIFY_RESOURCE(rsrc);

#define ZEND_FETCH_RESOURCE2_NO_RETURN(rsrc, rsrc_type, passed_id, default_id, resource_type_name, resource_type1, resource_type2)  \
  (rsrc = (rsrc_type) zend_fetch_resource(passed_id TSRMLS_CC, default_id, resource_type_name, NULL, 2, resource_type1, resource_type2))

#define ZEND_REGISTER_RESOURCE(rsrc_result, rsrc_pointer, rsrc_type)  \
    zend_register_resource(rsrc_result, rsrc_pointer, rsrc_type TSRMLS_CC);
END_EXTERN_C()

#endif
