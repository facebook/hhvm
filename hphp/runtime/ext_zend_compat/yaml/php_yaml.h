/**
 * YAML parser and emitter PHP extension
 *
 * Copyright (c) 2007 Ryusuke SEKIYAMA. All rights reserved.
 * Copyright (c) 2009 Keynetics Inc. All rights reserved.
 * Copyright (c) 2012 Bryan Davis All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any personobtaining a
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

#ifndef PHP_YAML_H
#define PHP_YAML_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <php.h>
#include <php_ini.h>
#include <SAPI.h>
#ifdef ZTS
#  include "TSRM.h"
#endif
#include <ext/standard/info.h>
#include <ext/standard/base64.h>
#include <ext/standard/basic_functions.h>
#include <ext/standard/php_var.h>
#include <ext/standard/php_smart_str.h>
#include <Zend/zend_extensions.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_interfaces.h>

#include <yaml.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PHP_YAML_MODULE_VERSION "1.1.1-dev"

extern zend_module_entry yaml_module_entry;
#define phpext_yaml_ptr &yaml_module_entry

/* {{{ module globals */

ZEND_BEGIN_MODULE_GLOBALS(yaml)
  zend_bool decode_binary;
  long decode_timestamp;
  zval *timestamp_decoder;
  zend_bool output_canonical;
  long output_indent;
  long output_width;
#ifdef IS_UNICODE
  UConverter *orig_runtime_encoding_conv;
#endif
ZEND_END_MODULE_GLOBALS(yaml)

ZEND_EXTERN_MODULE_GLOBALS(yaml)
#ifdef ZTS
#  define YAML_G(v) TSRMG(yaml_globals_id, zend_yaml_globals *, v)
#else
#  define YAML_G(v) (yaml_globals.v)
#endif
/* }}} */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PHP_YAML_H */
