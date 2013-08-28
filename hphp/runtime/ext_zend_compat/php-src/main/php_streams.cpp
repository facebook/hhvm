#include "hphp/runtime/ext_zend_compat/php-src/main/php.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/url-file.h"

PHPAPI size_t _php_stream_copy_to_mem(php_stream *src, char **buf, size_t maxlen, int persistent STREAMS_DC TSRMLS_DC) {
    HPHP::String s;
    if (maxlen == PHP_STREAM_COPY_ALL) {
      HPHP::StringBuffer sb;
      sb.read(src->hphp_file);
      s = sb.detach();
    } else {
      s = src->hphp_file->read(maxlen);
    }
    *buf = (char*) emalloc(s->size());
    memcpy(*buf, s->data(), s->size());
    return s->size();
}

PHPAPI int _php_stream_cast(php_stream *stream, int castas, void **ret, int show_err TSRMLS_DC) {
  switch (castas) {
    case PHP_STREAM_AS_STDIO:
      HPHP::PlainFile* pf = dynamic_cast<HPHP::PlainFile*>(stream->hphp_file);
      *ret = pf->getStream();
      return true;
  }
  return false;
}

PHPAPI php_stream *_php_stream_open_wrapper_ex(char *path, const char *mode, int options, char **opened_path, php_stream_context *context STREAMS_DC TSRMLS_DC) {
  HPHP::Stream::Wrapper* w = HPHP::Stream::getWrapperFromURI(path);
  HPHP::File* file = w->open(path, mode, options, context);
  if (!file) {
    return nullptr;
  }
  php_stream *stream = new (HPHP::request_arena()) php_stream(file);
  stream->hphp_file->incRefCount();

  if (auto urlFile = dynamic_cast<HPHP::UrlFile*>(file)) {
    // Why is there no ZVAL_ARRAY?
    MAKE_STD_ZVAL(stream->wrapperdata);
    Z_TYPE_P(stream->wrapperdata) = IS_ARRAY;
    Z_ARRVAL_P(stream->wrapperdata) = urlFile->getWrapperMetaData().detach();
  } else {
    stream->wrapperdata = nullptr;
  }

  return stream;
}
