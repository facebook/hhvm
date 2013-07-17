/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_BASE_INCLUDES_H_
#define incl_HPHP_BASE_INCLUDES_H_

///////////////////////////////////////////////////////////////////////////////
// headers needed by extensions.

#include "hphp/util/lock.h"
#include "hphp/runtime/base/hphp_system.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/util/compatibility.h"
#ifdef HHVM_FBMAKE
#include "hphp/system/constants.h"
#else
#include "system/constants.h"
#endif
///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_BASE_INCLUDES_H_
