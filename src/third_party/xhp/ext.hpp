/*
  +----------------------------------------------------------------------+
  | XHP                                                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
  | Copyright (c) 2009 - 2010 Facebook, Inc. (http://www.facebook.com)          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.00 of the Zend license,     |
  | that is bundled with this package in the file LICENSE.ZEND, and is   |
  | available through the world-wide-web at the following url:           |
  | http://www.zend.com/license/2_00.txt.                                |
  | If you did not receive a copy of the Zend license and are unable to  |
  | obtain it through the world-wide-web, please send a note to          |
  | license@zend.com so we can mail you a copy immediately.              |
  +----------------------------------------------------------------------+
*/

#pragma once
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#include "php.h"

#define PHP_XHP_VERSION "1.3.7"
#define PHP_XHP_EXTNAME "xhp"

extern zend_module_entry xhp_module_entry;
#define phpext_xhp &xhp_module_entry
