#include "zend_execute.h"

ZEND_API int zend_lookup_class(const char *name, int name_length, zend_class_entry ***ce TSRMLS_DC) {
  HPHP::String sname(name, name_length - 1, HPHP::CopyString);
  **ce = HPHP::Unit::loadClass(sname.get());
  return **ce == nullptr ? FAILURE : SUCCESS;
}

ZEND_API const char *get_active_class_name(const char **space TSRMLS_DC) {
  HPHP::Class *cls = HPHP::liveClass();
  if (!cls) {
    if (space) {
      *space = "";
    }
    return "";
  }
  if (space) {
    *space = "::";
  }
  return cls->name()->data();
}
