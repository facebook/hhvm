/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/phpfpm/ext_phpfpm.h"
#include "hphp/runtime/base/execution-context.h"

namespace HPHP {

bool HHVM_FUNCTION(fastcgi_finish_request) {
  auto context = g_context.getNoCheck();
  auto transport = context->getTransport();
  if (!transport) {
      return false;
  }
  context->obFlushAll();
  String content = context->obDetachContents();

  // User may use fastcgi_finish_request in shutdown functions
  // Shutdown functions will be invoked even if fatal error occurs
  int errnum = g_context->getLastErrorNumber();
  if (errnum == static_cast<int>(ErrorMode::FATAL_ERROR)){
    if (RuntimeOption::ServerErrorMessage) {
      String lastError = g_context->getLastError();
      transport->sendString(lastError.data(),
        500, false, false, "hphp_invoke");
    } else {
      transport->sendString(RuntimeOption::FatalErrorMessage,
        500, false, false, "hphp_invoke");
    }
  } else {
    transport->sendRaw((void*)content.data(), content.size());
  }
  transport->onSendEnd();
  return true;
}

class PHPFpmExtension : public Extension {
 public:
  PHPFpmExtension() : Extension("cgi-fcgi", "1.0.0") {}
  void moduleInit() override {
    HHVM_FE(fastcgi_finish_request);
    loadSystemlib("phpfpm");
  }
} s_phpfpm_extension;

}
