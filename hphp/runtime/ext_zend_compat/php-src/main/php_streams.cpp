#include "hphp/runtime/ext_zend_compat/php-src/main/php_streams.h"

PHPAPI size_t _php_stream_copy_to_mem(php_stream *src, char **buf, size_t maxlen, int persistent STREAMS_DC TSRMLS_DC) {
    HPHP::String s;
    if (maxlen == PHP_STREAM_COPY_ALL) {
      HPHP::StringBuffer sb;
      sb.read(src);
      s = sb.detach();
    } else {
      s = src->read(maxlen);
    }
    *buf = const_cast<char*>(s->data());
    return s->size();
}

PHPAPI int _php_stream_cast(php_stream *stream, int castas, void **ret, int show_err TSRMLS_DC) {
  switch (castas) {
    case PHP_STREAM_AS_STDIO:
      HPHP::PlainFile* pf = dynamic_cast<HPHP::PlainFile*>(stream);
      *ret = pf->getStream();
      return true;
  }
  return false;
}

PHPAPI php_stream *_php_stream_open_wrapper_ex(char *path, char *mode, int options, char **opened_path, php_stream_context *context STREAMS_DC TSRMLS_DC) {
  HPHP::Variant v = HPHP::File::Open(path, mode, options, context);
  php_stream *stream = dynamic_cast<php_stream*>(v.getResourceData());
  stream->incRefCount();
  return stream;
}
