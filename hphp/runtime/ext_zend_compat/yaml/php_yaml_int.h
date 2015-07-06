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
 * @copyright   2012 Bryan Davis
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 * @version     SVN: $Id$
 */


#ifndef PHP_YAML_INT_H
#define PHP_YAML_INT_H

#include "hphp/runtime/base/req-containers.h"

#ifdef __cplusplus
extern "C" {
#endif

/* {{{ ext/yaml types
*/
typedef zval *(*eval_scalar_func_t)(yaml_event_t event, HashTable *callbacks TSRMLS_DC);

typedef struct parser_state_s {
  yaml_parser_t parser;
  yaml_event_t event;
  int have_event;
  zval *aliases;
  eval_scalar_func_t eval_func;
  HashTable *callbacks;
} parser_state_t;

typedef struct y_emit_state_s {
  yaml_emitter_t *emitter;
  HashTable *recursive;
  HashTable *callbacks;
  HPHP::req::hash_map<void*, int, HPHP::pointer_hash<void> > seen;
} y_emit_state_t;

/* }}} */


/* {{{ ext/yaml macros
*/
#define YAML_BINARY_TAG     "tag:yaml.org,2002:binary"
#define YAML_MERGE_TAG      "tag:yaml.org,2002:merge"
#define YAML_PHP_TAG        "!php/object"

#define Y_SCALAR_IS_NOT_NUMERIC 0x00
#define Y_SCALAR_IS_INT         0x10
#define Y_SCALAR_IS_FLOAT       0x20
#define Y_SCALAR_IS_ZERO        0x00
#define Y_SCALAR_IS_BINARY      0x01
#define Y_SCALAR_IS_OCTAL       0x02
#define Y_SCALAR_IS_DECIMAL     0x03
#define Y_SCALAR_IS_HEXADECIMAL 0x04
#define Y_SCALAR_IS_SEXAGECIMAL 0x05
#define Y_SCALAR_IS_INFINITY_P  0x06
#define Y_SCALAR_IS_INFINITY_N  0x07
#define Y_SCALAR_IS_NAN         0x08
#define Y_SCALAR_FORMAT_MASK    0x0F


#if (PHP_MAJOR_VERSION > 5) || ((PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION >= 3))
#  define ZEND_IS_CALLABLE(a,b,c) zend_is_callable((a), (b), (c) TSRMLS_CC)
#else
#  define ZEND_IS_CALLABLE(a,b,c) zend_is_callable((a), (b), (c))
#endif

#define STR_EQ(a, b)\
  (0 == strcmp(a, b))

#define SCALAR_TAG_IS(event, name) \
  STR_EQ((const char *)event.data.scalar.tag, name)

#define IS_NOT_IMPLICIT_AND_TAG_IS(event, name) \
  (!event.data.scalar.quoted_implicit && !event.data.scalar.plain_implicit && SCALAR_TAG_IS(event, name))

#define IS_NOT_QUOTED_OR_TAG_IS(event, name) \
  ((YAML_PLAIN_SCALAR_STYLE == event.data.scalar.style || YAML_ANY_SCALAR_STYLE == event.data.scalar.style) && (event.data.scalar.plain_implicit || SCALAR_TAG_IS(event, name)))

/* }}} */

/* {{{ ext/yaml prototypes
*/
zval *php_yaml_read_all(parser_state_t *state, long *ndocs TSRMLS_DC);

zval *php_yaml_read_partial(
    parser_state_t *state, long pos, long *ndocs TSRMLS_DC);

zval *eval_scalar(yaml_event_t event, HashTable *callbacks TSRMLS_DC);

zval *eval_scalar_with_callbacks(
    yaml_event_t event, HashTable *callbacks TSRMLS_DC);

const char *detect_scalar_type(
    const char *value, size_t length, const yaml_event_t *event);

int scalar_is_null(
    const char *value, size_t length, const yaml_event_t *event);

int scalar_is_bool(
    const char *value, size_t length, const yaml_event_t *event);

int scalar_is_numeric(
    const char *value, size_t length, long *lval, double *dval, char **str);

int scalar_is_timestamp(const char *value, size_t length);

int php_yaml_write_impl(yaml_emitter_t *emitter, zval *data,
    yaml_encoding_t encoding, HashTable *callbacks TSRMLS_DC);

int php_yaml_write_to_buffer(
    void *data, unsigned char *buffer, size_t size);

/* }}} */

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* PHP_YAML_INT_H */
