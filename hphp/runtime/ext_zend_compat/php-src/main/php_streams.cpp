#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/ext_zend_compat/php-src/main/php.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/url-file.h"
#include "hphp/runtime/base/plain-file.h"

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
  // TODO this leaks
  php_stream *stream = new (HPHP::request_arena()) php_stream(file);
  stream->hphp_file->incRefCount();

  if (auto urlFile = dynamic_cast<HPHP::UrlFile*>(file)) {
    // Why is there no ZVAL_ARRAY?
    MAKE_STD_ZVAL(stream->wrapperdata);
    Z_TYPE_P(stream->wrapperdata) = IS_ARRAY;
    Z_ARRVAL_P(stream->wrapperdata) = HPHP::ProxyArray::Make(
      urlFile->getWrapperMetaData().detach()
    );
  } else {
    stream->wrapperdata = nullptr;
  }

  return stream;
}

PHPAPI int _php_stream_free(php_stream *stream, int close_options TSRMLS_DC) {
  decRefRes(stream->hphp_file);
  return 1;
}

PHPAPI int _php_stream_seek(php_stream *stream, off_t offset, int whence TSRMLS_DC) {
  return stream->hphp_file->seek(offset, whence);
}
 
PHPAPI off_t _php_stream_tell(php_stream *stream TSRMLS_DC) {
  return stream->hphp_file->tell();
}

PHPAPI size_t _php_stream_read(php_stream *stream, char *buf, size_t count TSRMLS_DC) {
  return stream->hphp_file->readImpl(buf, count);
}

PHPAPI size_t _php_stream_write(php_stream *stream, const char *buf, size_t count TSRMLS_DC) {
  return stream->hphp_file->writeImpl(buf, count);
}

PHPAPI int _php_stream_eof(php_stream *stream TSRMLS_DC) {
  return stream->hphp_file->eof();
}

PHPAPI int _php_stream_getc(php_stream *stream TSRMLS_DC) {
  return stream->hphp_file->getc();
}

PHPAPI int _php_stream_putc(php_stream *stream, int c TSRMLS_DC) {
  return stream->hphp_file->putc(c);
}
