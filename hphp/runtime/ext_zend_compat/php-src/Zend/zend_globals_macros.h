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
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef ZEND_GLOBALS_MACROS_H
#define ZEND_GLOBALS_MACROS_H

/* Executor */
# define EG(v) EG_##v()

zval*& EG_exception();
zval*& EG_prev_exception();
HashTable& EG_regular_list();
HashTable& EG_persistent_list();
zend_error_handling_t& EG_error_handling();
HashTable* EG_function_table();

#endif /* ZEND_GLOBALS_MACROS_H */
