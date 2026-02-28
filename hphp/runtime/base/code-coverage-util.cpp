/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/code-coverage-util.h"

#include "hphp/runtime/base/execution-context.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool isEnableCodeCoverageReqParamTrue() {
    auto const tport = g_context->getTransport();
    return tport &&
            tport->getParam("enable_code_coverage").compare("true") == 0;
}

bool isEnablePerFileCoverageReqParamTrue() {
    auto const tport = g_context->getTransport();
    return tport &&
            tport->getParam("enable_per_file_coverage").compare("true") == 0;
}

///////////////////////////////////////////////////////////////////////////////
}
