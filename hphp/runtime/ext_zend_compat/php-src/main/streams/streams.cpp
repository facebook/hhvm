/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Wez Furlong <wez@thebrainroom.com>                          |
   | Borrowed code from:                                                  |
   |          Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Jim Winstead <jimw@php.net>                                 |
   +----------------------------------------------------------------------+
 */
/* @nolint */
/* $Id$ */

#include "php.h"
#include "php_globals.h"
#include "php_network.h"
#include "ext/standard/file.h"
#include "ext/standard/php_string.h" /* for php_memnstr, used by php_stream_get_record() */
#include <stddef.h>
#include <fcntl.h>
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/url-file.h"
#include "hphp/runtime/base/plain-file.h"

/* {{{ resource and registration code */
static int le_stream = FAILURE; /* true global */
static int le_pstream = FAILURE; /* true global */
static int le_stream_filter = FAILURE; /* true global */

PHPAPI int php_file_le_stream(void)
{
  return le_stream;
}

PHPAPI int php_file_le_pstream(void)
{
  return le_pstream;
}

PHPAPI int php_file_le_stream_filter(void)
{
  return le_stream_filter;
}

PHPAPI int php_stream_context_set_option(php_stream_context *context,
    const char *wrappername, const char *optionname, zval *optionvalue)
{
  zval **wrapperhash;
  zval *category, *copied_val;

  ALLOC_INIT_ZVAL(copied_val);
  *copied_val = *optionvalue;
  zval_copy_ctor(copied_val);
  INIT_PZVAL(copied_val);

  if (FAILURE == zend_hash_find(Z_ARRVAL_P(context->options), (char*)wrappername, strlen(wrappername)+1, (void**)&wrapperhash)) {
    MAKE_STD_ZVAL(category);
    array_init(category);
    if (FAILURE == zend_hash_update(Z_ARRVAL_P(context->options), (char*)wrappername, strlen(wrappername)+1, (void**)&category, sizeof(zval *), NULL)) {
      return FAILURE;
    }

    wrapperhash = &category;
  }
  return zend_hash_update(Z_ARRVAL_PP(wrapperhash), (char*)optionname, strlen(optionname)+1, (void**)&copied_val, sizeof(zval *), NULL);
}
/* }}} */

PHPAPI php_stream_context *php_stream_context_alloc(TSRMLS_D)
{
  php_stream_context *context;

  context = (php_stream_context*) ecalloc(1, sizeof(php_stream_context));
  context->notifier = NULL;
  MAKE_STD_ZVAL(context->options);
  array_init(context->options);

  context->rsrc_id = ZEND_REGISTER_RESOURCE(NULL, context, php_le_stream_context(TSRMLS_C));
  return context;
}

/* allocate a new stream for a particular ops */
PHPAPI php_stream *_php_stream_alloc(php_stream_ops *ops, void *abstract, const char *persistent_id, const char *mode STREAMS_DC TSRMLS_DC) /* {{{ */
{
  php_stream *ret;

  ret = (php_stream*) pemalloc(sizeof(php_stream), persistent_id ? 1 : 0);

  memset(ret, 0, sizeof(php_stream));

  ret->readfilters.stream = ret;
  ret->writefilters.stream = ret;

#if STREAM_DEBUG
  fprintf(stderr, "stream_alloc: %s:%p persistent=%s\n", ops->label, ret, persistent_id);
#endif

  ret->ops = ops;
  ret->abstract = abstract;
  ret->is_persistent = persistent_id ? 1 : 0;
  ret->chunk_size = FG(def_chunk_size);

  if (FG(auto_detect_line_endings)) {
    ret->flags |= PHP_STREAM_FLAG_DETECT_EOL;
  }

  if (persistent_id) {
    auto le = HPHP::newres<zend_rsrc_list_entry>(ret, le_pstream);
    SCOPE_EXIT { delete le; };
    le->refcount = 0;

    if (FAILURE == zend_hash_update(&EG(persistent_list), (char *)persistent_id,
          strlen(persistent_id) + 1,
          (void*)le, sizeof(*le), NULL)) {
      pefree(ret, 1);
      return NULL;
    }
  }

  ret->rsrc_id = ZEND_REGISTER_RESOURCE(NULL, ret, persistent_id ? le_pstream : le_stream);
  strlcpy(ret->mode, mode, sizeof(ret->mode));

  ret->wrapper          = NULL;
  ret->wrapperthis      = NULL;
  ret->wrapperdata      = NULL;
  ret->stdiocast        = NULL;
  ret->orig_path        = NULL;
  ret->context          = NULL;
  ret->readbuf          = NULL;
  ret->enclosing_stream = NULL;

  return ret;
}
/* }}} */

/* {{{ php_stream_locate_url_wrapper */
PHPAPI php_stream_wrapper *php_stream_locate_url_wrapper(const char *path, char **path_for_open, int options TSRMLS_DC) {
  assert(options == 0);
  // TODO this leaks
  auto w = HPHP::Stream::getWrapperFromURI(path);
  if (!w) {
    return nullptr;
  }
  // fill this out if people need it
  php_stream_wrapper *stream = HPHP::req::make_raw<php_stream_wrapper>();
  return stream;
}

PHPAPI int _php_stream_stat(php_stream *stream, php_stream_statbuf *ssb TSRMLS_DC) {
  // kinda weird this is on the wrapper not the file
  auto path = stream->hphp_file->getName();
  auto w = HPHP::Stream::getWrapperFromURI(path);
  return w ? w->stat(path, &ssb->sb) : -1;
}

/* If buf == NULL, the buffer will be allocated automatically and will be of an
 * appropriate length to hold the line, regardless of the line length, memory
 * permitting */
PHPAPI char *_php_stream_get_line(php_stream *stream, char *buf, size_t maxlen,
    size_t *returned_len TSRMLS_DC)
{
  auto s = stream->hphp_file->readLine(maxlen);
  if (s.empty()) {
    return nullptr;
  }
  if (!buf) {
    buf = (char*) emalloc(s.length() + 1);
  }
  memcpy(buf, s.data(), s.length());
  buf[s.length()] = '\0';
  *returned_len = s.length();
  return buf;
}

PHPAPI php_stream *_php_stream_opendir(char *path, int options, php_stream_context *context STREAMS_DC TSRMLS_DC)
{
  auto wrapper = HPHP::Stream::getWrapperFromURI(path);
  if (!wrapper) {
    return nullptr;
  }
  auto dir = wrapper->opendir(path);
  if (!dir) {
    return nullptr;
  }

  // TODO this leaks
  php_stream *stream = HPHP::req::make_raw<php_stream>(dir.get());
  stream->hphp_dir->incRefCount();
  return stream;
}

PHPAPI php_stream_dirent *_php_stream_readdir(php_stream *dirstream, php_stream_dirent *ent TSRMLS_DC)
{
  auto *dir = dirstream->hphp_dir;
  if (!dir) {
    return nullptr;
  }
  auto v = dir->read();
  if (!v.isString()) {
    return nullptr;
  }
  auto s = v.toString();
  if (!s) {
    return nullptr;
  }
  memcpy(ent, s.data(), s.size());
  return ent;
}

PHPAPI int _php_stream_set_option(php_stream *stream, int option, int value, void *ptrparam TSRMLS_DC)
{
  throw HPHP::FatalErrorException("unimplemented: _php_stream_set_option");
}

PHPAPI php_stream_context *php_stream_context_set(php_stream *stream, php_stream_context *context)
{
  throw HPHP::FatalErrorException("unimplemented: php_stream_context_set");
}

PHPAPI void php_stream_notification_notify(php_stream_context *context, int notifycode, int severity,
    char *xmsg, int xcode, size_t bytes_sofar, size_t bytes_max, void * ptr TSRMLS_DC)
{
  throw HPHP::FatalErrorException("unimplemented: php_stream_notification_notify");
}

PHPAPI void php_stream_context_free(php_stream_context *context)
{
  throw HPHP::FatalErrorException("unimplemented: php_stream_context_free");
}

PHPAPI php_stream_notifier *php_stream_notification_alloc(void)
{
  throw HPHP::FatalErrorException("unimplemented: php_stream_notification_alloc");
}

PHPAPI void php_stream_notification_free(php_stream_notifier *notifier)
{
  throw HPHP::FatalErrorException("unimplemented: php_stream_notification_free");
}

PHPAPI int php_stream_context_get_option(php_stream_context *context,
    const char *wrappername, const char *optionname, zval ***optionvalue)
{
  throw HPHP::FatalErrorException("unimplemented: php_stream_context_get_option");
}

PHPAPI size_t _php_stream_copy_to_mem(php_stream *src, char **buf, size_t maxlen, int persistent STREAMS_DC TSRMLS_DC) {
    HPHP::String s;
    if (maxlen == PHP_STREAM_COPY_ALL) {
      HPHP::StringBuffer sb;
      sb.read(src->hphp_file);
      s = sb.detach();
    } else {
      s = src->hphp_file->read(maxlen);
    }
    *buf = (char*) emalloc(s.size());
    memcpy(*buf, s.data(), s.size());
    return s.size();
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
  if (!w) return nullptr;
  // This was using the implicit Variant(bool) ctor.
  // TODO: fixing this properly requires D1787768.
  auto file = w->open(path, mode, options, nullptr/*context*/);
  if (!file) {
    return nullptr;
  }
  // TODO this leaks
  php_stream *stream = HPHP::req::make_raw<php_stream>(file.get());
  stream->hphp_file->incRefCount();

  if (auto urlFile = dynamic_cast<HPHP::UrlFile*>(file.get())) {
    // Why is there no ZVAL_ARRAY?
    MAKE_STD_ZVAL(stream->wrapperdata);
    Z_TYPE_P(stream->wrapperdata) = IS_ARRAY;
    Z_ARRVAL_P(stream->wrapperdata) = HPHP::ProxyArray::Make(
      urlFile->getWrapperMetaData().getArrayData()
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
