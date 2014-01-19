/**
 * YAML parser and emitter PHP extension
 *
 * Copyright (c) 2007 Ryusuke SEKIYAMA. All rights reserved.
 * Copyright (c) 2009 Keynetics Inc. All rights reserved.
 * Copyright (c) 2012 Bryan Davis All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * @package     php_yaml
 * @author      Ryusuke SEKIYAMA <rsky0711@gmail.com>
 * @author      Bryan Davis <bpd@keynetics.com>
 * @copyright   2007 Ryusuke SEKIYAMA
 * @copyright   2009 Keynetics Inc
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 * @version     SVN: $Id$
 */

#include "php_yaml.h"
#include "zval_refcount.h"    /* for PHP < 5.3 */
#include "php_yaml_int.h"

/* {{{ local macros
 */
#if ZEND_EXTENSION_API_NO < 220060519
#define PHP_GINIT_FUNCTION(yaml) \
  void php_yaml_init_globals(zend_yaml_globals *yaml_globals)
#endif

/* }}} */


/* {{{ local prototypes
 */
static int php_yaml_check_callbacks(HashTable *callbacks TSRMLS_DC);

static PHP_MINIT_FUNCTION(yaml);
static PHP_MSHUTDOWN_FUNCTION(yaml);
static PHP_MINFO_FUNCTION(yaml);

static PHP_GINIT_FUNCTION(yaml);

PHP_FUNCTION(yaml_parse);
PHP_FUNCTION(yaml_parse_file);
PHP_FUNCTION(yaml_parse_url);
PHP_FUNCTION(yaml_emit);
PHP_FUNCTION(yaml_emit_file);

/* }}} */

/* {{{ globals */

ZEND_DECLARE_MODULE_GLOBALS(yaml);

/* }}} */

/* {{{ ini entries */

#ifndef ZEND_ENGINE_2
#define OnUpdateLong OnUpdateInt
#endif

PHP_INI_BEGIN()
  STD_PHP_INI_ENTRY("yaml.decode_binary", "0", PHP_INI_ALL, OnUpdateBool,
      decode_binary, zend_yaml_globals, yaml_globals)
  STD_PHP_INI_ENTRY("yaml.decode_timestamp", "0", PHP_INI_ALL, OnUpdateLong,
      decode_timestamp, zend_yaml_globals, yaml_globals)
  STD_PHP_INI_ENTRY("yaml.output_canonical", "0", PHP_INI_ALL, OnUpdateBool,
      output_canonical, zend_yaml_globals, yaml_globals)
  STD_PHP_INI_ENTRY("yaml.output_indent", "2", PHP_INI_ALL, OnUpdateLong,
      output_indent, zend_yaml_globals, yaml_globals)
  STD_PHP_INI_ENTRY("yaml.output_width", "80", PHP_INI_ALL, OnUpdateLong,
      output_width, zend_yaml_globals, yaml_globals)
PHP_INI_END()

/* }}} */


/* {{{ argument information */
#ifdef ZEND_BEGIN_ARG_INFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_yaml_parse, ZEND_SEND_BY_VAL,
  ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, input)
  ZEND_ARG_INFO(0, pos)
  ZEND_ARG_INFO(1, ndocs)
  ZEND_ARG_ARRAY_INFO(0, callbacks, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_yaml_parse_file, ZEND_SEND_BY_VAL,
  ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, filename)
  ZEND_ARG_INFO(0, pos)
  ZEND_ARG_INFO(1, ndocs)
  ZEND_ARG_ARRAY_INFO(0, callbacks, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_yaml_parse_url, ZEND_SEND_BY_VAL,
  ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, url)
  ZEND_ARG_INFO(0, pos)
  ZEND_ARG_INFO(1, ndocs)
  ZEND_ARG_ARRAY_INFO(0, callbacks, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_yaml_emit, ZEND_SEND_BY_VAL,
  ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, data)
  ZEND_ARG_INFO(0, encoding)
  ZEND_ARG_INFO(0, linebreak)
  ZEND_ARG_ARRAY_INFO(0, callbacks, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_yaml_emit_file, ZEND_SEND_BY_VAL,
  ZEND_RETURN_VALUE, 2)
  ZEND_ARG_INFO(0, filename)
  ZEND_ARG_INFO(0, data)
  ZEND_ARG_INFO(0, encoding)
  ZEND_ARG_INFO(0, linebreak)
  ZEND_ARG_ARRAY_INFO(0, callbacks, 0)
ZEND_END_ARG_INFO()
#else
#define arginfo_yaml_parse third_arg_force_ref
#define arginfo_yaml_parse_file third_arg_force_ref
#define arginfo_yaml_parse_url third_arg_force_ref
#endif

static zend_function_entry yaml_functions[] = {
  PHP_FE(yaml_parse, arginfo_yaml_parse)
  PHP_FE(yaml_parse_file, arginfo_yaml_parse_file)
  PHP_FE(yaml_parse_url, arginfo_yaml_parse_url)
  PHP_FE(yaml_emit, arginfo_yaml_emit)
  PHP_FE(yaml_emit_file, arginfo_yaml_emit_file)
  {NULL, NULL, NULL}
};
/* }}} */


/* {{{ cross-extension dependencies */

#if ZEND_EXTENSION_API_NO >= 220050617
static zend_module_dep yaml_deps[] = {
  ZEND_MOD_OPTIONAL("date") {NULL, NULL, NULL, 0}
};
#endif
/* }}} */


zend_module_entry yaml_module_entry = {  /* {{{ */
#if ZEND_EXTENSION_API_NO >= 220050617
  STANDARD_MODULE_HEADER_EX,
  NULL,
  yaml_deps,
#else
  STANDARD_MODULE_HEADER,
#endif
  "yaml",
  yaml_functions,
  PHP_MINIT(yaml),
  PHP_MSHUTDOWN(yaml),
  NULL, /* RINIT */
  NULL, /* RSHUTDOWN */
  PHP_MINFO(yaml),
  PHP_YAML_MODULE_VERSION,
#if ZEND_EXTENSION_API_NO >= 220060519
  PHP_MODULE_GLOBALS(yaml),
  PHP_GINIT(yaml),
  NULL,
  NULL,
  STANDARD_MODULE_PROPERTIES_EX
#else
  STANDARD_MODULE_PROPERTIES
#endif
};
/* }}} */


#ifdef COMPILE_DL_YAML
ZEND_GET_MODULE(yaml)
#endif


/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINIT_FUNCTION(yaml)
{
#if ZEND_EXTENSION_API_NO < 220060519
  ZEND_INIT_MODULE_GLOBALS(yaml, php_yaml_init_globals, NULL)
#endif
  REGISTER_INI_ENTRIES();

  /* node style constants */
  REGISTER_LONG_CONSTANT("YAML_ANY_SCALAR_STYLE",
      YAML_ANY_SCALAR_STYLE, CONST_PERSISTENT | CONST_CS);
  REGISTER_LONG_CONSTANT("YAML_PLAIN_SCALAR_STYLE",
      YAML_PLAIN_SCALAR_STYLE, CONST_PERSISTENT | CONST_CS);
  REGISTER_LONG_CONSTANT("YAML_SINGLE_QUOTED_SCALAR_STYLE",
      YAML_SINGLE_QUOTED_SCALAR_STYLE, CONST_PERSISTENT | CONST_CS);
  REGISTER_LONG_CONSTANT("YAML_DOUBLE_QUOTED_SCALAR_STYLE",
      YAML_DOUBLE_QUOTED_SCALAR_STYLE, CONST_PERSISTENT | CONST_CS);
  REGISTER_LONG_CONSTANT("YAML_LITERAL_SCALAR_STYLE",
      YAML_LITERAL_SCALAR_STYLE, CONST_PERSISTENT | CONST_CS);
  REGISTER_LONG_CONSTANT("YAML_FOLDED_SCALAR_STYLE",
      YAML_FOLDED_SCALAR_STYLE, CONST_PERSISTENT | CONST_CS);

  /* tag constants */
  REGISTER_STRING_CONSTANT("YAML_NULL_TAG", YAML_NULL_TAG,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_STRING_CONSTANT("YAML_BOOL_TAG", YAML_BOOL_TAG,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_STRING_CONSTANT("YAML_STR_TAG", YAML_STR_TAG,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_STRING_CONSTANT("YAML_INT_TAG", YAML_INT_TAG,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_STRING_CONSTANT("YAML_FLOAT_TAG", YAML_FLOAT_TAG,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_STRING_CONSTANT("YAML_TIMESTAMP_TAG", YAML_TIMESTAMP_TAG,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_STRING_CONSTANT("YAML_SEQ_TAG", YAML_SEQ_TAG,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_STRING_CONSTANT("YAML_MAP_TAG", YAML_MAP_TAG,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_STRING_CONSTANT("YAML_PHP_TAG", YAML_PHP_TAG,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_STRING_CONSTANT("YAML_MERGE_TAG", YAML_MERGE_TAG,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_STRING_CONSTANT("YAML_BINARY_TAG", YAML_BINARY_TAG,
      CONST_PERSISTENT | CONST_CS);

  /* encoding constants */
  REGISTER_LONG_CONSTANT("YAML_ANY_ENCODING", YAML_ANY_ENCODING,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_LONG_CONSTANT("YAML_UTF8_ENCODING", YAML_UTF8_ENCODING,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_LONG_CONSTANT("YAML_UTF16LE_ENCODING", YAML_UTF16LE_ENCODING,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_LONG_CONSTANT("YAML_UTF16BE_ENCODING", YAML_UTF16BE_ENCODING,
      CONST_PERSISTENT | CONST_CS);

  /* linebreak constants */
  REGISTER_LONG_CONSTANT("YAML_ANY_BREAK", YAML_ANY_BREAK,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_LONG_CONSTANT("YAML_CR_BREAK", YAML_CR_BREAK,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_LONG_CONSTANT("YAML_LN_BREAK", YAML_LN_BREAK,
      CONST_PERSISTENT | CONST_CS);
  REGISTER_LONG_CONSTANT("YAML_CRLN_BREAK", YAML_CRLN_BREAK,
      CONST_PERSISTENT | CONST_CS);

  return SUCCESS;
}
/* }}} */


/* {{{ PHP_MSHUTDOWN_FUNCTION */
static PHP_MSHUTDOWN_FUNCTION(yaml)
{
  UNREGISTER_INI_ENTRIES();
  return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(yaml)
{
  php_info_print_table_start();
  php_info_print_table_row(2, "LibYAML Support", "enabled");
  php_info_print_table_row(2, "Module Version", PHP_YAML_MODULE_VERSION);
  php_info_print_table_row(2, "LibYAML Version", yaml_get_version_string());
  php_info_print_table_end();

  DISPLAY_INI_ENTRIES();
}
/* }}} */


/* {{{ PHP_GINIT_FUNCTION() */
static PHP_GINIT_FUNCTION(yaml)
{
  yaml_globals->decode_binary = 0;
  yaml_globals->decode_timestamp = 0;
  yaml_globals->timestamp_decoder = NULL;
  yaml_globals->output_canonical = 0;
  yaml_globals->output_indent = 2;
  yaml_globals->output_width = 80;
#ifdef IS_UNICODE
  yaml_globals->orig_runtime_encoding_conv = NULL;
#endif
}
/* }}} */


/* {{{ php_yaml_check_callbacks()
 * Validate user supplied callback array contents
 */
static int php_yaml_check_callbacks(HashTable *callbacks TSRMLS_DC)
{
  zval **entry = { 0 };
#ifdef IS_UNICODE
  zstr key;
#else
  char *key = { 0 };
#endif
  uint key_len = 0;
  ulong idx = 0L;

  zend_hash_internal_pointer_reset(callbacks);

  while (SUCCESS ==
      zend_hash_get_current_data(callbacks, (void **) &entry)) {
    int key_type = zend_hash_get_current_key_ex(
        callbacks, &key, &key_len, &idx, 0, NULL);

#ifdef IS_UNICODE
    if (key_type == HASH_KEY_IS_STRING || key_type == HASH_KEY_IS_UNICODE) {
      zval name;
      int type =
        (key_type == HASH_KEY_IS_STRING) ? IS_STRING : IS_UNICODE;

      INIT_ZVAL(name);

      if (!zend_is_callable(*entry, 0, &name)) {
        if (Z_TYPE(name) == IS_UNICODE ||
          Z_TYPE(name) == IS_STRING) {
          php_error_docref(NULL TSRMLS_CC, E_WARNING,
              "Callback for tag '%R', '%R' is not valid",
              type, key, Z_TYPE(name), Z_UNIVAL(name));

        } else {
          php_error_docref(NULL TSRMLS_CC, E_WARNING,
              "Callback for tag '%R' is not valid",
              type, key);
        }

        zval_dtor(&name);
        return FAILURE;
      }

      if (ZEND_U_EQUAL(type, key, key_len - 1,
            YAML_TIMESTAMP_TAG, sizeof(YAML_TIMESTAMP_TAG) - 1)) {
        YAML_G(timestamp_decoder) = *entry;
      }

      zval_dtor(&name);

    } else {
      php_error_docref(NULL TSRMLS_CC, E_NOTICE,
          "Callback key should be a string");
    }
#else
    if (key_type == HASH_KEY_IS_STRING) {
      char *name;

      if (!ZEND_IS_CALLABLE(*entry, 0, &name)) {
        if (name != NULL) {
          php_error_docref(NULL TSRMLS_CC, E_WARNING,
              "Callback for tag '%s', '%s' is not valid",
              key, name);
          efree(name);

        } else {
          php_error_docref(NULL TSRMLS_CC, E_WARNING,
              "Callback for tag '%s' is not valid", key);
        }
        return FAILURE;
      }

      if (!strcmp(key, YAML_TIMESTAMP_TAG)) {
        YAML_G(timestamp_decoder) = *entry;
      }

      if (name != NULL) {
        efree(name);
      }

    } else {
      php_error_docref(NULL TSRMLS_CC, E_NOTICE,
          "Callback key should be a string");
    }
#endif

    zend_hash_move_forward(callbacks);
  }

  return SUCCESS;
}
/* }}} */


/* {{{ proto mixed yaml_parse(string input[, int pos[, int &ndocs[, array callbacks]]])
 Takes a YAML encoded string and converts it to a PHP variable. */
PHP_FUNCTION(yaml_parse)
{
  char *input = { 0 };
  int input_len = 0;
  long pos = 0;
  zval *zndocs = { 0 };
  zval *zcallbacks = { 0 };

  parser_state_t state;
  zval *yaml = { 0 };
  long ndocs = 0;

  memset(&state, 0, sizeof(state));
  state.have_event = 0;
  state.aliases = NULL;
  state.callbacks = NULL;

#ifdef IS_UNICODE
  YAML_G(orig_runtime_encoding_conv) = UG(runtime_encoding_conv);
#endif
  YAML_G(timestamp_decoder) = NULL;

#ifdef IS_UNICODE
  if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
          "s&|lza/", &input, &input_len, UG(utf8_conv), &pos,
          &zndocs, &zcallbacks)) {
    return;
  }
#else
  if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
          "s|lza/", &input, &input_len, &pos, &zndocs,
          &zcallbacks)) {
    return;
  }
#endif

  if (zcallbacks != NULL) {
    state.callbacks = Z_ARRVAL_P(zcallbacks);
    if (FAILURE == php_yaml_check_callbacks(state.callbacks TSRMLS_CC)) {
      RETURN_FALSE;
    }

    state.eval_func = eval_scalar_with_callbacks;

  } else {
    state.eval_func = eval_scalar;
  }

#ifdef IS_UNICODE
  UG(runtime_encoding_conv) = UG(utf8_conv);
#endif

  yaml_parser_initialize(&state.parser);
  yaml_parser_set_input_string(
      &state.parser, (unsigned char *) input, (size_t) input_len);


  if (pos < 0) {
    yaml = php_yaml_read_all(&state, &ndocs TSRMLS_CC);

  } else {
    yaml = php_yaml_read_partial(&state, pos, &ndocs TSRMLS_CC);
  }

  yaml_parser_delete(&state.parser);

#ifdef IS_UNICODE
  UG(runtime_encoding_conv) = YAML_G(orig_runtime_encoding_conv);
#endif

  if (zndocs != NULL) {
    /* copy document count to var user sent in */
    zval_dtor(zndocs);
    ZVAL_LONG(zndocs, ndocs);
  }

  if (NULL == yaml) {
    RETURN_FALSE;
  }

  RETURN_ZVAL(yaml, 1, 1);
}
/* }}} yaml_parse */


/* {{{ proto mixed yaml_parse_file(string filename[, int pos[, int &ndocs[, array callbacks]]])
   */
PHP_FUNCTION(yaml_parse_file)
{
  char *filename = { 0 };
  int filename_len = 0;
  long pos = 0;
  zval *zndocs = { 0 };
  zval *zcallbacks = { 0 };

  php_stream *stream = { 0 };
  FILE *fp = { 0 };

  parser_state_t state;
  zval *yaml = { 0 };
  long ndocs = 0;

  memset(&state, 0, sizeof(state));
  state.have_event = 0;
  state.aliases = NULL;
  state.callbacks = NULL;

#ifdef IS_UNICODE
  YAML_G(orig_runtime_encoding_conv) = UG(runtime_encoding_conv);
#endif
  YAML_G(timestamp_decoder) = NULL;

#ifdef IS_UNICODE
  if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
          "s&|lza/", &filename, &filename_len,
          ZEND_U_CONVERTER(UG(filesystem_encoding_conv)), &pos,
          &zndocs, &zcallbacks)) {
    return;
  }
#else
  if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
          "s|lza/", &filename, &filename_len, &pos, &zndocs,
          &zcallbacks)) {
    return;
  }
#endif

  if (zcallbacks != NULL) {
    state.callbacks = Z_ARRVAL_P(zcallbacks);
    if (FAILURE == php_yaml_check_callbacks(state.callbacks TSRMLS_CC)) {
      RETURN_FALSE;
    }

    state.eval_func = eval_scalar_with_callbacks;

  } else {
    state.eval_func = eval_scalar;
  }

  if (NULL == (stream = php_stream_open_wrapper(filename, "rb",
      IGNORE_URL | ENFORCE_SAFE_MODE | REPORT_ERRORS | STREAM_WILL_CAST,
      NULL))) {
    RETURN_FALSE;
  }

  if (FAILURE == php_stream_cast(
      stream, PHP_STREAM_AS_STDIO, (void **) &fp, 1)) {
    php_stream_close(stream);
    RETURN_FALSE;
  }

#ifdef IS_UNICODE
  UG(runtime_encoding_conv) = UG(utf8_conv);
#endif

  yaml_parser_initialize(&state.parser);
  yaml_parser_set_input_file(&state.parser, fp);

  if (pos < 0) {
    yaml = php_yaml_read_all(&state, &ndocs TSRMLS_CC);

  } else {
    yaml = php_yaml_read_partial(&state, pos, &ndocs TSRMLS_CC);
  }

  yaml_parser_delete(&state.parser);
  php_stream_close(stream);

#ifdef IS_UNICODE
  UG(runtime_encoding_conv) = YAML_G(orig_runtime_encoding_conv);
#endif

  if (zndocs != NULL) {
    /* copy document count to var user sent in */
    zval_dtor(zndocs);
    ZVAL_LONG(zndocs, ndocs);
  }

  if (NULL == yaml) {
    RETURN_FALSE;
  }

  RETURN_ZVAL(yaml, 1, 1);
}
/* }}} yaml_parse_file */


/* {{{ proto mixed yaml_parse_url(string url[, int pos[, int &ndocs[, array callbacks]]])
   */
PHP_FUNCTION(yaml_parse_url)
{
  char *url = { 0 };
  int url_len = 0;
  long pos = 0;
  zval *zndocs = { 0 };
  zval *zcallbacks = { 0 };

  php_stream *stream = { 0 };
  char *input = { 0 };
  size_t size = 0;

  parser_state_t state;
  zval *yaml = { 0 };
  long ndocs = 0;

  memset(&state, 0, sizeof(state));
  state.have_event = 0;
  state.aliases = NULL;
  state.callbacks = NULL;

#ifdef IS_UNICODE
  YAML_G(orig_runtime_encoding_conv) = UG(runtime_encoding_conv);
#endif
  YAML_G(timestamp_decoder) = NULL;

  if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
        "s|lza/", &url, &url_len, &pos, &zndocs, &zcallbacks)) {
    return;
  }

  if (zcallbacks != NULL) {
    state.callbacks = Z_ARRVAL_P(zcallbacks);
    if (FAILURE == php_yaml_check_callbacks(state.callbacks TSRMLS_CC)) {
      RETURN_FALSE;
    }

    state.eval_func = eval_scalar_with_callbacks;

  } else {
    state.eval_func = eval_scalar;
  }

  if (NULL == (stream = php_stream_open_wrapper(url, "rb",
          ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL))) {
    RETURN_FALSE;
  }

#ifdef IS_UNICODE
  size = php_stream_copy_to_mem(
      stream, (void **) &input, PHP_STREAM_COPY_ALL, 0);
#else
  size = php_stream_copy_to_mem(stream, &input, PHP_STREAM_COPY_ALL, 0);
#endif

#ifdef IS_UNICODE
  UG(runtime_encoding_conv) = UG(utf8_conv);
#endif

  yaml_parser_initialize(&state.parser);
  yaml_parser_set_input_string(&state.parser, (unsigned char *)input, size);

  if (pos < 0) {
    yaml = php_yaml_read_all(&state, &ndocs TSRMLS_CC);

  } else {
    yaml = php_yaml_read_partial(&state, pos, &ndocs TSRMLS_CC);
  }

  yaml_parser_delete(&state.parser);
  php_stream_close(stream);
  efree(input);

#ifdef IS_UNICODE
  UG(runtime_encoding_conv) = YAML_G(orig_runtime_encoding_conv);
#endif

  if (zndocs != NULL) {
    /* copy document count to var user sent in */
    zval_dtor(zndocs);
    ZVAL_LONG(zndocs, ndocs);
  }

  if (NULL == yaml) {
    RETURN_FALSE;
  }

  RETURN_ZVAL(yaml, 1, 1);
}
/* }}} yaml_parse_url */


/* {{{ proto string yaml_emit(mixed data[, int encoding[, int linebreak[, array callbacks]]])
   */
PHP_FUNCTION(yaml_emit)
{
  zval *data = { 0 };
  long encoding = YAML_ANY_ENCODING;
  long linebreak = YAML_ANY_BREAK;
  zval *zcallbacks = { 0 };
  HashTable *callbacks = { 0 };

  yaml_emitter_t emitter = { YAML_NO_ERROR };
  smart_str str = { 0 };

  if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "z/|lla/",
        &data, &encoding, &linebreak, &zcallbacks)) {
    return;
  }

  if (zcallbacks != NULL) {
    callbacks = Z_ARRVAL_P(zcallbacks);
    if (FAILURE == php_yaml_check_callbacks(callbacks TSRMLS_CC)) {
      RETURN_FALSE;
    }
  }

  yaml_emitter_initialize(&emitter);
  yaml_emitter_set_output(
      &emitter, &php_yaml_write_to_buffer, (void *) &str);
  yaml_emitter_set_encoding(&emitter, (yaml_encoding_t) encoding);
  yaml_emitter_set_break(&emitter, (yaml_break_t) linebreak);
  yaml_emitter_set_canonical(&emitter, YAML_G(output_canonical));
  yaml_emitter_set_indent(&emitter, YAML_G(output_indent));
  yaml_emitter_set_width(&emitter, YAML_G(output_width));
  yaml_emitter_set_unicode(&emitter, YAML_ANY_ENCODING != encoding);

  if (SUCCESS == php_yaml_write_impl(
        &emitter, data, (yaml_encoding_t) encoding,
        callbacks TSRMLS_CC)) {
#ifdef IS_UNICODE
    RETVAL_U_STRINGL(UG(utf8_conv), str.c, str.len, ZSTR_DUPLICATE);
#else
    RETVAL_STRINGL(str.c, str.len, 1);
#endif

  } else {
    RETVAL_FALSE;
  }

  yaml_emitter_delete(&emitter);
  smart_str_free(&str);
}
/* }}} yaml_emit */



/* {{{ proto bool yaml_emit_file(string filename, mixed data[, string encoding[, string linebreak[, array callbacks]]])
   */
PHP_FUNCTION(yaml_emit_file)
{
  char *filename = { 0 };
  int filename_len = 0;
  php_stream *stream = { 0 };
  FILE *fp = { 0 };
  zval *data = { 0 };
  const char *encoding = { 0 };
  int encoding_len = 0;
  const char *linebreak = { 0 };
  int linebreak_len = 0;
  zval *zcallbacks = { 0 };
  HashTable *callbacks = { 0 };

  yaml_emitter_t emitter = { YAML_NO_ERROR };

#ifdef IS_UNICODE
  if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s&z/|ssa/",
      &filename, &filename_len,
      ZEND_U_CONVERTER(UG(filesystem_encoding_conv)), &data,
      &encoding, &encoding_len, &linebreak, &zcallbacks,
      &linebreak_len)) {
    return;
  }
#else
  if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz/|ssa/",
      &filename, &filename_len, &data, &encoding,
      &encoding_len, &linebreak, &zcallbacks,
      &linebreak_len)) {
    return;
  }
#endif

  php_error_docref(NULL TSRMLS_CC, E_WARNING, "not yet implemented");
  RETURN_FALSE;

  if (NULL == (stream = php_stream_open_wrapper(filename, "wb",
      IGNORE_URL | ENFORCE_SAFE_MODE | REPORT_ERRORS | STREAM_WILL_CAST,
      NULL))) {
    RETURN_FALSE;
  }

  if (FAILURE == php_stream_cast(
      stream, PHP_STREAM_AS_STDIO, (void **) &fp, 1)) {
    php_stream_close(stream);
    RETURN_FALSE;
  }

  yaml_emitter_initialize(&emitter);
  yaml_emitter_set_output_file(&emitter, fp);
  yaml_emitter_set_encoding(&emitter, (yaml_encoding_t) (size_t) encoding);
  yaml_emitter_set_break(&emitter, (yaml_break_t) (size_t) linebreak);
  yaml_emitter_set_canonical(&emitter, YAML_G(output_canonical));
  yaml_emitter_set_indent(&emitter, YAML_G(output_indent));
  yaml_emitter_set_width(&emitter, YAML_G(output_width));
  yaml_emitter_set_unicode(&emitter, YAML_ANY_ENCODING != (size_t) encoding);

  RETVAL_BOOL((SUCCESS == php_yaml_write_impl(
      &emitter, data, YAML_ANY_ENCODING, callbacks TSRMLS_CC)));

  yaml_emitter_delete(&emitter);
  php_stream_close(stream);
}
/* }}} yaml_emit_file */
