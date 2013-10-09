/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_RESERVED_STACK_H_
#define incl_HPHP_VM_RESERVED_STACK_H_

/*
 * RESERVED_STACK_TOTAL_SPACE space (in bytes) at 8(%rsp) is
 * allocated on entry to the TC and made available for scratch
 * purposes (right above the return address).  It is used as spill
 * locations (see LinearScan), and for MInstrState.
 */
#define RESERVED_STACK_MINSTR_STATE_SPACE 0x80
#define RESERVED_STACK_SPILL_SPACE        0x400
#define RESERVED_STACK_TOTAL_SPACE        (RESERVED_STACK_MINSTR_STATE_SPACE + \
                                           RESERVED_STACK_SPILL_SPACE)

#endif
