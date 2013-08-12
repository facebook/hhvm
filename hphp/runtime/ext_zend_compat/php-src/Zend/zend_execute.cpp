#include "zend_execute.h"

ZEND_API int zend_lookup_class(const char *name, int name_length, zend_class_entry ***ce TSRMLS_DC) {
  HPHP::StringData *class_name = HPHP::StringData::GetStaticString(name, name_length);
  **ce = HPHP::Unit::loadClass(class_name);
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
