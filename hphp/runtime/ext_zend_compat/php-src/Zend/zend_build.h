/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2013 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Stanislav Malyshev <stas@zend.com>                          |
   +----------------------------------------------------------------------+
*/

#ifndef ZEND_BUILD_H
#define ZEND_BUILD_H

#define ZEND_TOSTR_(x) #x
#define ZEND_TOSTR(x) ZEND_TOSTR_(x)

#define ZEND_BUILD_TS ",NTS"

#define ZEND_BUILD_DEBUG

#define ZEND_BUILD_SYSTEM

/* for private applications */
#define ZEND_BUILD_EXTRA

#endif
