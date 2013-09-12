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

/* $Id$ */

#include "php.h"
#include "php_globals.h"
#include "php_network.h"
#include "ext/standard/file.h"
#include "ext/standard/php_string.h" /* for php_memnstr, used by php_stream_get_record() */
#include <stddef.h>
#include <fcntl.h>

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
		zend_rsrc_list_entry le;

		le.type = le_pstream;
		le.ptr = ret;
		le.refcount = 0;

		if (FAILURE == zend_hash_update(&EG(persistent_list), (char *)persistent_id,
					strlen(persistent_id) + 1,
					(void *)&le, sizeof(le), NULL)) {

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

