/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef HPHP_DWARF_MSVC_H_
#define HPHP_DWARF_MSVC_H_

// We don't have libdwarf, so stub it out.
using Dwarf_Ptr = void*;
using Dwarf_Unsigned = uint32_t;

#define DW_CFA_set_loc 0

#define DW_CFA_def_cfa 0

#define DW_CFA_same_value 0
#define DW_CFA_offset 0
#define DW_CFA_offset_extended_sf 0

#define DW_EH_PE_absptr 0

#endif
