/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   +----------------------------------------------------------------------+
 */

// TODO(#3704) This should be removed

#ifndef incl_XDEBUG_MM_H_
#define incl_XDEBUG_MM_H_

#include "hphp/runtime/base/memory-manager.h"

#define xdmalloc    HPHP::req::malloc
#define xdcalloc    HPHP::req::calloc
#define xdrealloc   HPHP::req::realloc
#define xdfree      HPHP::req::free

#endif
