/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef PHP_PACKET_SOAP_H
#define PHP_PACKET_SOAP_H

#include "hphp/runtime/ext/soap/sdl.h"
#include <memory>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SoapClient;
bool parse_packet_soap(SoapClient *client,
                       const char *buffer,
                       int buffer_size,
                       std::shared_ptr<sdlFunction> fn,
                       const char *fn_name,
                       Variant& return_value,
                       Array& soap_headers);

///////////////////////////////////////////////////////////////////////////////
}

#endif
