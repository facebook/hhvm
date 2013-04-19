/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
namespace HPHP {

bool fh_use_soap_error_handler(bool handler) asm("_ZN4HPHP24f_use_soap_error_handlerEb");

bool fh_is_soap_fault(TypedValue* fault) asm("_ZN4HPHP15f_is_soap_faultERKNS_7VariantE");

long fh__soap_active_version() asm("_ZN4HPHP22f__soap_active_versionEv");

} // namespace HPHP
