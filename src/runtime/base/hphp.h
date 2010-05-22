/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __CPP_BASE_HPHP_H__
#define __CPP_BASE_HPHP_H__

///////////////////////////////////////////////////////////////////////////////

/**
 * This is the file that's included at top of a code generated program.
 *
 * Do NOT include this file in runtime/base, as this depends on a generated
 * globals.h that defines GlobalVariables class.
 */

#include <runtime/base/base_includes.h>
#include <runtime/base/program_functions.h>
#include <runtime/ext/ext.h>
#include <runtime/eval/eval.h>

///////////////////////////////////////////////////////////////////////////////

#endif // __CPP_BASE_HPHP_H__
