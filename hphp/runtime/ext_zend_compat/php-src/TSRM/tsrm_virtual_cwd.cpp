#include "hphp/runtime/ext_zend_compat/php-src/TSRM/tsrm_virtual_cwd.h"

#include "hphp/runtime/ext/ext_file.h"

CWD_API char *tsrm_realpath(const char *path, char *real_path TSRMLS_DC) {
  HPHP::Variant rp = HPHP::f_realpath(path);
  if (rp.isBoolean()) {
    assert(!rp.toBoolean());
    return nullptr;
  }

  HPHP::StringData *ret = rp.getStringData();

  if (real_path) {
    int copy_len = ret->size();
    memcpy(real_path, ret->data(), copy_len);
    real_path[copy_len] = '\0';
    return real_path;
  } else {
    return strndup(ret->data(), ret->size());
  }
}
