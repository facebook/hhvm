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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
namespace {

  Array make_parameters_dict(const Transport::ParamMap& params) {
    DictInit ret(params.size());
    for (auto iter = params.begin(); iter != params.end(); ++iter) {
      VecArrayInit values(iter->second.size());
      for (
        auto viter = iter->second.begin();
        viter != iter->second.end();
        ++viter
      ) {
        values.append(*viter);
      }
      ret.set(String(iter->first), values.toArray());
    }
    return ret.toArray();
  }

  Array HHVM_FUNCTION(http_request_get_parameters) {
    Transport* transport = g_context->getTransport();
    if (UNLIKELY(!transport)) {
      return Array::CreateDict();
    }
    const auto params = transport->getQueryParams();
    return make_parameters_dict(params);
  }

  Array HHVM_FUNCTION(http_request_post_parameters) {
    Transport* transport = g_context->getTransport();
    if (UNLIKELY(!transport)) {
      return Array::CreateDict();
    }
    const auto params = transport->getPostParams();
    return make_parameters_dict(params);
  }

  struct HSLHTTPRequestExtension final : Extension {

    HSLHTTPRequestExtension() : Extension("hsl_httprequest", "1.0") {}

    void moduleInit() override {
      HHVM_FALIAS(
        HH\\Lib\\_Private\\Native\\http_request_get_parameters,
        http_request_get_parameters
      );
      HHVM_FALIAS(
        HH\\Lib\\_Private\\Native\\http_request_post_parameters,
        http_request_post_parameters
      );
      loadSystemlib();
    }
  } s_hsl_http_request_extension;

} // anonymous namespace
} // namespace HPHP
