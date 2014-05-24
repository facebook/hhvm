#ifndef incl_ZEND_EXTENSION_INLINE_H_
#define incl_ZEND_EXTENSION_INLINE_H_

namespace HPHP {

inline void ZendExtension::moduleShutdown() {
  if (!RuntimeOption::EnableZendCompat) {
    return;
  }
  zend_module_entry* entry = getEntry();
  if (entry->module_shutdown_func) {
    TSRMLS_FETCH();
    entry->module_shutdown_func(1, 1 TSRMLS_CC);
  }
}
inline void ZendExtension::threadShutdown() {
  if (!RuntimeOption::EnableZendCompat) {
    return;
  }
  zend_module_entry* entry = getEntry();
  if (entry->globals_size) {
    ts_free_id(*entry->globals_id_ptr);
  }
}
inline void ZendExtension::requestInit() {
  if (!RuntimeOption::EnableZendCompat) {
    return;
  }
  zend_module_entry* entry = getEntry();
  if (entry->request_startup_func) {
    TSRMLS_FETCH();
    entry->request_startup_func(0, entry->module_number TSRMLS_CC);
  }
}
inline void ZendExtension::requestShutdown() {
  if (!RuntimeOption::EnableZendCompat) {
    return;
  }
  zend_module_entry* entry = getEntry();
  if (entry->request_shutdown_func) {
    TSRMLS_FETCH();
    entry->request_shutdown_func(0, entry->module_number TSRMLS_CC);
  }
  // TODO: call these after all RSHUTDOWN functions from other modules have been called
  if (entry->post_deactivate_func) {
    entry->post_deactivate_func();
  }
}
inline zend_module_entry *ZendExtension::getEntry() {
  enum { offset = offsetof(struct _zend_module_entry, name) };
  return (zend_module_entry*)(((char*)this) - offset);
}

}
#endif
