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
#define Y_PARSER_CONTINUE 0
#define Y_PARSER_SUCCESS  1
#define Y_PARSER_FAILURE -1

#define Y_FILTER_NONE     0
#define Y_FILTER_SUCCESS  1
#define Y_FILTER_FAILURE -1

#define NEXT_EVENT() yaml_next_event(state TSRMLS_CC)

#define COPY_EVENT(dest, state) \
  memcpy(&dest, &state->event, sizeof(yaml_event_t)); \
  state->have_event = 0; \
  memset(&state->event, 0, sizeof(yaml_event_t))


#ifdef IS_UNICODE
#define MAKE_ARRAY(var) \
  MAKE_STD_ZVAL(var); \
  array_init(var); \
  Z_ARRVAL_P(var)->unicode = UG(unicode)
#else
#define MAKE_ARRAY(var) \
  MAKE_STD_ZVAL(var); \
  array_init(var)
#endif
/* }}} */


/* {{{ local prototypes
 */
static void handle_parser_error(const yaml_parser_t *parser TSRMLS_DC);

static inline int yaml_next_event(parser_state_t *state TSRMLS_DC);

static zval *get_next_element(parser_state_t *state TSRMLS_DC);

static zval *handle_document(parser_state_t *state TSRMLS_DC);

static zval *handle_mapping(parser_state_t *state TSRMLS_DC);

static zval *handle_sequence(parser_state_t *state TSRMLS_DC);

static zval *handle_scalar(parser_state_t *state TSRMLS_DC);

static zval *handle_alias(parser_state_t *state TSRMLS_DC);

static int apply_filter(
    zval **zpp, yaml_event_t event, HashTable *callbacks TSRMLS_DC);

static char *convert_to_char(zval *zv TSRMLS_DC);

static int eval_timestamp(zval **zpp, char *ts, int ts_len TSRMLS_DC);

/* }}} */


/* {{{ php_yaml_read_all()
 * Process events from yaml parser
 */
zval *php_yaml_read_all(parser_state_t *state, long *ndocs TSRMLS_DC)
{
  zval *retval = { 0 };
  zval *doc = { 0 };
  int code = Y_PARSER_CONTINUE;

  /* create an empty array to hold results */
  MAKE_ARRAY(retval);

  while (Y_PARSER_CONTINUE == code) {

    if (!NEXT_EVENT()) {
      code = Y_PARSER_FAILURE;
      break;

    } else if (YAML_STREAM_END_EVENT == state->event.type) {
      code = Y_PARSER_SUCCESS;
      break;

    } else if (YAML_STREAM_START_EVENT == state->event.type) {
      if (!NEXT_EVENT()) {
        code = Y_PARSER_FAILURE;
        break;
      }

    }

    if (YAML_DOCUMENT_START_EVENT != state->event.type) {
      code = Y_PARSER_FAILURE;

      php_error_docref(NULL TSRMLS_CC, E_WARNING,
          "expected DOCUMENT_START event, got %d "
          "(line %zd, column %zd)",
          state->event.type,
          state->parser.mark.line + 1,
          state->parser.mark.column + 1);
      break;
    }

    doc = handle_document(state TSRMLS_CC);

    if (NULL == doc) {
      code = Y_PARSER_FAILURE;
      break;
    }

    add_next_index_zval(retval, doc);

    (*ndocs)++;
  }

  if (state->have_event) {
    yaml_event_delete(&state->event);
  }

  if (Y_PARSER_FAILURE == code) {
    zval_ptr_dtor(&retval);
    retval = NULL;
  }

  return retval;
}
/* }}} */


/* {{{ php_yaml_read_partial()
 * Read a particular document from the parser's document stream.
 */
zval *php_yaml_read_partial(
    parser_state_t *state, long pos, long *ndocs TSRMLS_DC)
{
  zval *retval = { 0 };
  int code = Y_PARSER_CONTINUE;

  while (Y_PARSER_CONTINUE == code) {

    if (!NEXT_EVENT()) {
      code = Y_PARSER_FAILURE;

    } else if (YAML_STREAM_END_EVENT == state->event.type) {
      /* reached end of stream without finding what we wanted */
      php_error_docref(NULL TSRMLS_CC, E_WARNING,
          "end of stream reached without finding document %ld",
          pos);
      code = Y_PARSER_FAILURE;

    } else if (YAML_DOCUMENT_START_EVENT == state->event.type) {
      if (*ndocs == pos) {
        retval = handle_document(state TSRMLS_CC);
        if (NULL == retval) {
          code = Y_PARSER_FAILURE;
          break;
        }
        code = Y_PARSER_SUCCESS;
      }
      /* count each document as we pass */
      (*ndocs)++;
    }
  }

  if (state->have_event) {
    yaml_event_delete(&state->event);
  }

  if (Y_PARSER_FAILURE == code) {
    if (NULL != retval) {
      zval_ptr_dtor(&retval);
    }
    retval = NULL;
  }

  return retval;
}
/* }}} */


/* {{{ handle_parser_error()
 * Emit a warning about a parser error
 */
static void handle_parser_error(const yaml_parser_t *parser TSRMLS_DC)
{
  const char *error_type;

  switch (parser->error) {
  case YAML_MEMORY_ERROR:
    error_type = "memory allocation";
    break;

  case YAML_READER_ERROR:
    error_type = "reading";
    break;

  case YAML_SCANNER_ERROR:
    error_type = "scanning";
    break;

  case YAML_PARSER_ERROR:
    error_type = "parsing";
    break;

  default:
    /* Shouldn't happen. */
    error_type = "unknown";
    break;
  }

  if (NULL != parser->problem) {
    if (parser->context) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING,
          "%s error encountered during parsing: %s "
          "(line %zd, column %zd), "
          "context %s (line %zd, column %zd)",
          error_type,
          parser->problem,
          parser->problem_mark.line + 1,
          parser->problem_mark.column + 1, parser->context,
          parser->context_mark.line + 1,
          parser->context_mark.column + 1);
    } else {
      php_error_docref(NULL TSRMLS_CC, E_WARNING,
          "%s error encountered during parsing: %s "
          "(line %zd, column %zd)",
          error_type,
          parser->problem,
          parser->problem_mark.line + 1,
          parser->problem_mark.column + 1);
    }
  } else {
    php_error_docref(NULL TSRMLS_CC, E_WARNING,
        "%s error encountred during parsing", error_type);
  }
}
/* }}} */


/* {{{ yaml_next_event()
 * Load the next parser event
 */
static inline int yaml_next_event(parser_state_t *state TSRMLS_DC)
{
  if (state->have_event) {
    /* free prior event */
    yaml_event_delete(&state->event);
    state->have_event = 0;
  }

  if (!yaml_parser_parse(&state->parser, &state->event)) {
    /* error encountered parsing input */
    state->have_event = 0;
    handle_parser_error(&state->parser TSRMLS_CC);

  } else {
    state->have_event = 1;
  }

  return state->have_event;
}
/* }}} */


/* {{{ get_next_element()
 * Extract the next whole element from the parse stream
 */
static zval *get_next_element(parser_state_t *state TSRMLS_DC)
{
  zval *retval = { 0 };

  if (!NEXT_EVENT()) {
    /* check state->event if you need to know the difference between
     * this error and a halt event
     */
    return NULL;
  }

  switch (state->event.type) {
  case YAML_DOCUMENT_END_EVENT:
  case YAML_MAPPING_END_EVENT:
  case YAML_SEQUENCE_END_EVENT:
  case YAML_STREAM_END_EVENT:
    /* halting events */
    break;

  case YAML_DOCUMENT_START_EVENT:
    retval = handle_document(state TSRMLS_CC);
    break;

  case YAML_MAPPING_START_EVENT:
    retval = handle_mapping(state TSRMLS_CC);
    break;

  case YAML_SEQUENCE_START_EVENT:
    retval = handle_sequence(state TSRMLS_CC);
    break;

  case YAML_SCALAR_EVENT:
    retval = handle_scalar(state TSRMLS_CC);
    break;

  case YAML_ALIAS_EVENT:
    retval = handle_alias(state TSRMLS_CC);
    break;

  default:
    /* any other event is an error */
    php_error_docref(NULL TSRMLS_CC, E_WARNING,
        "Unexpected event type %d "
        "(line %zd, column %zd)",
        state->event.type,
        state->parser.mark.line + 1,
        state->parser.mark.column + 1);
    break;
  }

  return retval;
}
/* }}} */


/* {{{ handle_document()
 * Handle a document event
 */
static zval *handle_document(parser_state_t *state TSRMLS_DC)
{
  zval *aliases = { 0 };
  zval *retval = { 0 };

  /* make a new array to hold aliases */
  MAKE_ARRAY(aliases);
  state->aliases = aliases;

  /* document consists of next element */
  retval = get_next_element(state TSRMLS_CC);

  /* clean up aliases */
  state->aliases = NULL;
  zval_ptr_dtor(&aliases);

  /* assert that end event is next in stream */
  if (NULL != retval && NEXT_EVENT() &&
      YAML_DOCUMENT_END_EVENT != state->event.type) {
    zval_ptr_dtor(&retval);
    retval = NULL;
  }

  return retval;
}
/* }}} */


/* {{{ handle_mapping()
 * Handle a mapping event
 */
static zval *handle_mapping(parser_state_t *state TSRMLS_DC)
{
  zval *retval = { 0 };
  yaml_event_t src_event = { YAML_NO_EVENT }, key_event = { YAML_NO_EVENT };
  zval *key = { 0 };
  char *key_str;
  zval *value = { 0 };

  /* save copy of mapping start event */
  COPY_EVENT(src_event, state);

  /* make a new array to hold mapping */
  MAKE_ARRAY(retval);

  if (NULL != src_event.data.mapping_start.anchor) {
    /* record anchors in current alias table */
    Z_ADDREF_P(retval);
    add_assoc_zval(state->aliases,
        (char *) src_event.data.mapping_start.anchor,
        retval);
  }

  while (NULL != (key = get_next_element(state TSRMLS_CC))) {
    COPY_EVENT(key_event, state);
    key_str = convert_to_char(key TSRMLS_CC);
    zval_ptr_dtor(&key);

    value = get_next_element(state TSRMLS_CC);

    if (NULL == value) {
      zval_ptr_dtor(&retval);
      yaml_event_delete(&src_event);
      efree(key_str);
      yaml_event_delete(&key_event);
      return NULL;
    }

    /* check for '<<' and handle merge */
    if (IS_NOT_QUOTED_OR_TAG_IS(key_event, YAML_MERGE_TAG) &&
        STR_EQ("<<", key_str)) {
      /* zend_hash_merge */
      /*
       * value is either a single ref or a simple array of refs
       */
      if (YAML_ALIAS_EVENT == state->event.type) {
        /* single ref */
        zend_hash_merge(Z_ARRVAL_P(retval), Z_ARRVAL_P(value),
            (void (*)(void *pData)) zval_add_ref,
            NULL, sizeof(zval*), 0);

      } else {
        /* array of refs */
        HashTable *ht = Z_ARRVAL_P(value);
        zval **zvalpp;

        zend_hash_internal_pointer_reset(ht);
        while (SUCCESS == zend_hash_has_more_elements(ht)) {
          zend_hash_get_current_data(ht, (void**)&zvalpp);
          zend_hash_merge(Z_ARRVAL_P(retval), Z_ARRVAL_PP(zvalpp),
              (void (*)(void *pData)) zval_add_ref,
            NULL, sizeof(zval*), 0);
          zend_hash_move_forward(ht);
        };
      }

      zval_ptr_dtor(&value);

    } else {

      /* add key => value to retval */
      add_assoc_zval(retval, key_str, value);
    }

    efree(key_str);
    yaml_event_delete(&key_event);
  }

  if (YAML_MAPPING_END_EVENT != state->event.type) {
    zval_ptr_dtor(&retval);
    retval = NULL;
  }

  if (NULL != retval && NULL != state->callbacks) {
    /* apply callbacks to the collected node */
    if (Y_FILTER_FAILURE == apply_filter(
        &retval, src_event, state->callbacks TSRMLS_CC)) {
      zval_ptr_dtor(&retval);
      retval = NULL;
    }
  }

  yaml_event_delete(&src_event);
  return retval;
}
/* }}} */


/* {{{ handle_sequence
 * Handle a sequence event
 */
static zval *handle_sequence (parser_state_t *state TSRMLS_DC) {
  zval *retval = { 0 };
  yaml_event_t src_event = { YAML_NO_EVENT };
  zval *value = { 0 };

  /* save copy of sequence start event */
  COPY_EVENT(src_event, state);

  /* make a new array to hold mapping */
  MAKE_ARRAY(retval);

  if (NULL != src_event.data.sequence_start.anchor) {
    /* record anchors in current alias table */
    Z_ADDREF_P(retval);
    add_assoc_zval(state->aliases,
        (char *) src_event.data.sequence_start.anchor,
        retval);
  }

  while (NULL != (value = get_next_element(state TSRMLS_CC))) {
    add_next_index_zval(retval, value);
  }

  if (YAML_SEQUENCE_END_EVENT != state->event.type) {
    zval_ptr_dtor(&retval);
    retval = NULL;
  }

  if (NULL != retval && NULL != state->callbacks) {
    /* apply callbacks to the collected node */
    if (Y_FILTER_FAILURE == apply_filter(
        &retval, src_event, state->callbacks TSRMLS_CC)) {
      zval_ptr_dtor(&retval);
      retval = NULL;
    }
  }

  yaml_event_delete(&src_event);
  return retval;
}
/* }}} */


/* {{{ handle_scalar()
 * Handle a scalar event
 */
static zval *handle_scalar(parser_state_t *state TSRMLS_DC) {
  zval *retval = { 0 };

  retval = state->eval_func(state->event, state->callbacks TSRMLS_CC);

  if (NULL != retval && NULL != state->event.data.scalar.anchor) {
    /* record anchors in current alias table */
    Z_ADDREF_P(retval);
    add_assoc_zval(state->aliases,
        (char *) state->event.data.scalar.anchor, retval);
  }
  return retval;
}
/* }}} */


/* {{{ handle_alias()
 * Handle an alias event
 */
static zval *handle_alias(parser_state_t *state TSRMLS_DC) {
  zval **retval = { 0 };
  char *anchor = (char *) state->event.data.alias.anchor;

  if (FAILURE == zend_hash_find(Z_ARRVAL_P(state->aliases),
      anchor, (uint) strlen(anchor) + 1,
      (void **) &retval)) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING,
        "alias %s is not registered "
        "(line %zd, column %zd)",
        anchor,
        state->parser.mark.line + 1,
        state->parser.mark.column + 1);

    return NULL;
  }

  /* add a reference to retval's internal counter */
  Z_ADDREF_PP(retval);
  Z_SET_ISREF_PP(retval);

  return (*retval);
}
/* }}} */




/* {{{ apply_filter()
 * Apply user supplied hander to node
 */
static int
apply_filter(zval **zpp, yaml_event_t event, HashTable *callbacks TSRMLS_DC)
{
  char *tag = { 0 };
  zval **callback = { 0 };

  /* detect event type and get tag */
  switch (event.type) {
  case YAML_SEQUENCE_START_EVENT:
    if (event.data.sequence_start.implicit) {
      tag = YAML_SEQ_TAG;
    } else {
      tag = (char *) event.data.sequence_start.tag;
    }
    break;

  case YAML_MAPPING_START_EVENT:
    if (event.data.sequence_start.implicit) {
      tag = YAML_MAP_TAG;
    } else {
      tag = (char *) event.data.mapping_start.tag;
    }
    break;

  default:
    /* don't care about other event types */
    break;
  }

  if (NULL == tag) {
    return Y_FILTER_NONE;
  }

  /* find and apply the filter function */
  if (SUCCESS == zend_hash_find(
      callbacks, tag, strlen(tag) + 1, (void **) &callback)) {
    int callback_result;
    zval **callback_args[] = { zpp, NULL, NULL };
    zval *tag_arg = { 0 };
    zval *mode_arg = { 0 };
    zval *retval_ptr = { 0 };

    MAKE_STD_ZVAL(tag_arg);
    ZVAL_STRINGL(tag_arg, tag, strlen(tag), 1);
    callback_args[1] = &tag_arg;

    MAKE_STD_ZVAL(mode_arg);
    ZVAL_LONG(mode_arg, 0);
    callback_args[2] = &mode_arg;

    /* call the user function */
    callback_result = call_user_function_ex(EG(function_table), NULL,
        *callback, &retval_ptr, 3, callback_args, 0, NULL TSRMLS_CC);

    /* cleanup our temp variables */
    zval_ptr_dtor(&tag_arg);
    zval_ptr_dtor(&mode_arg);

    if (FAILURE == callback_result || NULL == retval_ptr) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING,
          "Failed to apply filter for tag '%s'"
          " with user defined function", tag);
      return Y_FILTER_FAILURE;

    } else {
      if (retval_ptr == *zpp) {
        /* throw away duplicate response */
        zval_ptr_dtor(&retval_ptr);
      } else {
        /* copy result into our return var */
        REPLACE_ZVAL_VALUE(zpp, retval_ptr, 0);
      }
      return Y_FILTER_SUCCESS;
    }

  } else {
    return Y_FILTER_NONE;
  }
}
/* }}} */


/* {{{ eval_scalar()
 * Convert a scalar node to the proper PHP data type.
 *
 * All YAML scalar types found at http://yaml.org/type/index.html.
 */
zval *eval_scalar(yaml_event_t event,
    HashTable * callbacks TSRMLS_DC)
{
  zval *retval = { 0 };
  char *value = (char *) event.data.scalar.value;
  size_t length = event.data.scalar.length;
  int flags = 0;

  MAKE_STD_ZVAL(retval);
  ZVAL_NULL(retval);

  /* check for null */
  if (scalar_is_null(value, length, &event)) {
    return retval;
  }

  /* check for bool */
  if (-1 != (flags = scalar_is_bool(value, length, &event))) {
    ZVAL_BOOL(retval, (zend_bool) flags);
    return retval;
  }

  /* check for numeric (int or float) */
  if (!event.data.scalar.quoted_implicit &&
      (event.data.scalar.plain_implicit ||
       SCALAR_TAG_IS(event, YAML_INT_TAG) ||
       SCALAR_TAG_IS(event, YAML_FLOAT_TAG))) {
    long lval = 0;
    double dval = 0.0;

    flags = scalar_is_numeric(
        value, length, &lval, &dval, NULL);
    if (flags != Y_SCALAR_IS_NOT_NUMERIC) {
      if (flags & Y_SCALAR_IS_FLOAT) {
        ZVAL_DOUBLE(retval, dval);

      } else {
        ZVAL_LONG(retval, lval);
      }

      if (event.data.scalar.plain_implicit) {
        /* pass */

      } else if (SCALAR_TAG_IS(event, YAML_FLOAT_TAG) &&
          (flags & Y_SCALAR_IS_INT)) {
        convert_to_double(retval);

      } else if (SCALAR_TAG_IS(event, YAML_INT_TAG) &&
          (flags & Y_SCALAR_IS_FLOAT)) {
        convert_to_long(retval);
      }

      return retval;

    } else if (IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_FLOAT_TAG)) {
      ZVAL_STRINGL(retval, value, length, 1);
      convert_to_double(retval);
      return retval;

    } else if (IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_INT_TAG)) {
      ZVAL_STRINGL(retval, value, length, 1);
      convert_to_long(retval);
      return retval;
    }
  }

  /* check for timestamp */
  if (event.data.scalar.plain_implicit || event.data.scalar.quoted_implicit) {
    if (scalar_is_timestamp(value, length)) {
      if (FAILURE == eval_timestamp(
          &retval, value, (int) length TSRMLS_CC)) {
        zval_ptr_dtor(&retval);
        return NULL;
      }
      return retval;
    }

  } else if (SCALAR_TAG_IS(event, YAML_TIMESTAMP_TAG)) {
    if (FAILURE == eval_timestamp(
        &retval, value, (int) length TSRMLS_CC)) {
      zval_ptr_dtor(&retval);
      return NULL;
    }
    return retval;
  }

  /* check for binary */
  if (IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_BINARY_TAG)) {
    if (YAML_G(decode_binary)) {
      unsigned char *data = { 0 };
      int data_len = 0;

      data = php_base64_decode(
          (const unsigned char *) value, (int) length, &data_len);
      if (NULL == data) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
            "Failed to decode binary");
        ZVAL_NULL(retval);

      } else {
        ZVAL_STRINGL(retval, (char *) data, data_len, 0);
      }

    } else {
      ZVAL_STRINGL(retval, value, length, 1);
    }

    return retval;
  }

  /* check for php object */
  if (IS_NOT_IMPLICIT_AND_TAG_IS(event, YAML_PHP_TAG)) {
    const unsigned char *p;
    php_unserialize_data_t var_hash;

    p = (const unsigned char *) value;
    PHP_VAR_UNSERIALIZE_INIT(var_hash);

    if (!php_var_unserialize(
        &retval, &p, p + (int) length, &var_hash TSRMLS_CC)) {
      PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
      php_error_docref(NULL TSRMLS_CC, E_NOTICE,
          "Failed to unserialize class");
      /* return the serialized string directly */
      ZVAL_STRINGL(retval, value, length, 1);
    }

    PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
    return retval;
  }

  /* others (treat as a string) */
#ifdef IS_UNICODE
  ZVAL_U_STRINGL(UG(utf8_conv), retval, value, length, ZSTR_DUPLICATE);
#else
  ZVAL_STRINGL(retval, value, length, 1);
#endif

  return retval;
}
/* }}} */


/* {{{ eval_scalar_with_callbacks()
 * Convert a scalar node to the proper PHP data type using user supplied input
 * filters if available.
 */
zval *eval_scalar_with_callbacks(yaml_event_t event,
    HashTable *callbacks TSRMLS_DC)
{
  const char *tag = (char *) event.data.scalar.tag;
  zval **callback = { 0 };

  if (YAML_PLAIN_SCALAR_STYLE == event.data.scalar.style && NULL == tag) {
    /* plain scalar with no specified type */
    tag = detect_scalar_type(
        (char *) event.data.scalar.value, event.data.scalar.length,
        &event);
  }
  if (NULL == tag) {
    /* couldn't/wouldn't detect tag type, assume string */
    tag = YAML_STR_TAG;
  }

  /* find and apply the evaluation function */
  if (SUCCESS == zend_hash_find(
      callbacks, tag, strlen(tag) + 1, (void **) &callback)) {
    zval **argv[] = { NULL, NULL, NULL };
    zval *arg1 = { 0 };
    zval *arg2 = { 0 };
    zval *arg3 = { 0 };
    zval *retval = { 0 };

    MAKE_STD_ZVAL(arg1);
    ZVAL_STRINGL(arg1, (char *) event.data.scalar.value,
        event.data.scalar.length, 1);
    argv[0] = &arg1;

    MAKE_STD_ZVAL(arg2);
    ZVAL_STRINGL(arg2, tag, strlen(tag), 1);
    argv[1] = &arg2;

    MAKE_STD_ZVAL(arg3);
    ZVAL_LONG(arg3, event.data.scalar.style);
    argv[2] = &arg3;

    if (FAILURE == call_user_function_ex(EG(function_table), NULL,
        *callback, &retval, 3, argv, 0, NULL TSRMLS_CC) ||
        NULL == retval) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING,
          "Failed to evaluate value for tag '%s'"
          " with user defined function", tag);
    }

    zval_ptr_dtor(&arg1);
    zval_ptr_dtor(&arg2);
    zval_ptr_dtor(&arg3);

    return retval;
  }

  /* no mapping, so handle raw */
  return eval_scalar(event, NULL TSRMLS_CC);
}
/* }}} */


/* {{{ convert_to_char()
 * Convert a zval to a character array.
 */
static char *convert_to_char(zval *zv TSRMLS_DC)
{
  char *str = { 0 };

  switch (Z_TYPE_P(zv)) {
  case IS_BOOL:
    if (Z_BVAL_P(zv)) {
      str = estrndup("1", 1);
    } else {
      str = estrndup("", 0);
    }
    break;

  case IS_DOUBLE:
    {
      char buf[64] = { '\0' };

      (void) snprintf(buf, 64, "%G", Z_DVAL_P(zv));
      str = estrdup(buf);
    }
    break;

  case IS_LONG:
    {
      char buf[32] = { '\0' };

      (void) snprintf(buf, 32, "%ld", Z_LVAL_P(zv));
      str = estrdup(buf);
    }
    break;

  case IS_NULL:
    str = estrndup("", 0);
    break;

  case IS_STRING:
    str = estrndup(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
    break;

#ifdef IS_UNICODE
  case IS_UNICODE:
    {
      int len;
      UErrorCode status = U_ZERO_ERROR;

      zend_unicode_to_string_ex(UG(utf8_conv), &str, &len,
          Z_USTRVAL_P(zv), Z_USTRLEN_P(zv), &status);
      if (U_FAILURE(status)) {
        if (str != NULL) {
          efree(str);
          str = NULL;
        }
      }
    }
    break;
#endif

#ifdef ZEND_ENGINE_2
  case IS_OBJECT:
    {
      zval tmp;

      if (SUCCESS == zend_std_cast_object_tostring(
#if PHP_MAJOR_VERSION >= 6
          zv, &tmp, IS_STRING, UG(utf8_conv) TSRMLS_CC
#elif PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 2
          zv, &tmp, IS_STRING TSRMLS_CC
#else
          zv, &tmp, IS_STRING, 0 TSRMLS_CC
#endif
          )) {
        str = estrndup(Z_STRVAL(tmp), Z_STRLEN(tmp));
        zval_dtor(&tmp);
        return str;
      }
    }
#endif
    break;

  default:
    {
      php_serialize_data_t var_hash;
      smart_str buf = { 0 };

      PHP_VAR_SERIALIZE_INIT(var_hash);
      php_var_serialize(&buf, &zv, &var_hash TSRMLS_CC);
      PHP_VAR_SERIALIZE_DESTROY(var_hash);

      str = buf.c;
    }
    break;
  }

  if (NULL == str) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING,
        "Failed to convert %s to string", zend_zval_type_name(zv));
  }

  return str;
}
/* }}} */


/* {{{ eval_timestamp()
 * Convert a timestamp
 *
 * This is switched on/off by the `yaml.decode_timestamp` ini setting.
 *  - yaml.decode_timestamp=0 for no timestamp parsing
 *  - yaml.decode_timestamp=1 for strtotime parsing
 *  - yaml.decode_timestamp=2 for date_create parsing
 */
static int
eval_timestamp(zval ** zpp, char *ts, int ts_len TSRMLS_DC)
{
  if (NULL != YAML_G(timestamp_decoder) ||
      1L == YAML_G(decode_timestamp) ||
      2L == YAML_G(decode_timestamp)) {
    zval **argv[] = { NULL };
    zval *arg, *retval, *func, afunc;
    char *funcs[] = { "strtotime", "date_create" };

    INIT_ZVAL(afunc);
    if (NULL == YAML_G(timestamp_decoder)) {
      if (2L == YAML_G(decode_timestamp)) {
        ZVAL_STRING(&afunc, funcs[1], 0);

      } else {
        ZVAL_STRING(&afunc, funcs[0], 0);
      }

      func = &afunc;
    } else {
      func = YAML_G(timestamp_decoder);
    }

    MAKE_STD_ZVAL(arg);
#ifdef ZEND_ENGINE_2
    ZVAL_STRINGL(arg, ts, ts_len, 1);
#else
    {
      /* fix timestamp format for PHP4 */
      char *buf, *dst, *end, *src;

      buf = (char *) emalloc((size_t) ts_len + 1);
      dst = buf;
      end = ts + ts_len;
      src = ts;

      while (src < end && *src != '.') {
        if (src + 1 < end &&
            (*(src - 1) >= '0' && *(src - 1) <= '9') &&
            (*src == 'T' || *src == 't') &&
            (*(src + 1) >= '0' && *(src + 1) <= '9')) {
          src++;
          *dst++ = ' ';

        } else if (*src == ':' && src > ts + 2 && (
            ((*(src - 2) == '+' || *(src - 2) == '-') &&
             (*(src - 1) >= '0' || *(src - 1) <= '5')) ||
            ((*(src - 3) == '+' || *(src - 3) == '-') &&
             (*(src - 2) >= '0' || *(src - 2) <= '5') &&
             (*(src - 1) >= '0' || *(src - 1) <= '9')))) {
          src++;

        } else {
          *dst++ = *src++;
        }
      }

      if (src < end && *src == '.') {
        src++;
        while (src < end && *src >= '0' && *src <= '9') {
          src++;
        }
      }

      while (src < end) {
        if (*src == ':' && src > ts + 2 && (
            ((*(src - 2) == '+' || *(src - 2) == '-') &&
             (*(src - 1) >= '0' || *(src - 1) <= '5')) ||
            ((*(src - 3) == '+' || *(src - 3) == '-') &&
             (*(src - 2) >= '0' || *(src - 2) <= '5') &&
             (*(src - 1) >= '0' || *(src - 1) <= '9')))) {
          src++;

        } else {
          *dst++ = *src++;
        }
      }

      *dst = '\0';

      ZVAL_STRINGL(arg, buf, dst - buf, 0);
    }
#endif

    argv[0] = &arg;

    if (FAILURE == call_user_function_ex(EG(function_table), NULL, func,
        &retval, 1, argv, 0, NULL TSRMLS_CC) || NULL == retval) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING,
          "Failed to evaluate string '%s' as timestamp", ts);
      zval_ptr_dtor(&arg);
      return FAILURE;

    } else {
      zval_ptr_dtor(&arg);
      REPLACE_ZVAL_VALUE(zpp, retval, 0);
      return SUCCESS;
    }

  } else {
    zval_dtor(*zpp);
#ifdef IS_UNICODE
    ZVAL_U_STRINGL(UG(utf8_conv), *zpp, ts, ts_len, 1);
#else
    ZVAL_STRINGL(*zpp, ts, ts_len, 1);
#endif
    return SUCCESS;
  }
}
/* }}} */


