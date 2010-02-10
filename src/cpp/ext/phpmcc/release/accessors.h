/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#if !defined(__EXT_PHPMCC_ACCESSORS_H__)
#define __EXT_PHPMCC_ACCESSORS_H__

#include "types.h"

namespace HPHP {

extern Array get_serverpool_servers(mcc_handle_t mcc,
                                    const nstring_t* serverpool,
                                    bool is_mirror);
extern Array get_accesspoints(mcc_handle_t mcc, const nstring_t* server);
extern Variant phpmcc_read_property(MccResourcePtr &phpmcc, CVarRef member);
extern void phpmcc_write_property(MccResourcePtr &phpmcc, CVarRef member,
                                  CVarRef value);

///////////////////////////////////////////////////////////////////////////////
}

#endif /* #if !defined(__EXT_PHPMCC_ACCESSORS_H__) */
