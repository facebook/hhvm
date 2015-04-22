/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/xdebug/chrome.h"

#include "hphp/util/exception.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_xml.h"

#include <folly/json.h>
#include <folly/Range.h>

using folly::dynamic;
using folly::StringPiece;

namespace HPHP {
//////////////////////////////////////////////////////////////////////

namespace {
//////////////////////////////////////////////////////////////////////

/* Linked list search for matching attribute. */
xdebug_xml_attribute* find_attr(xdebug_xml_node* node, StringPiece name) {
  auto attr = node->attribute;

  while (attr != nullptr) {
    if (attr->name == name) {
      return attr;
    }
    attr = attr->next;
  }

  return nullptr;
}

//////////////////////////////////////////////////////////////////////
}

std::string chrome_to_dbgp(StringPiece chrome) {
  try {
    auto const dyn     = parseJson(chrome, folly::json::serialization_opts{});

    auto const& req    = dyn["request"];
    auto const& id     = req["id"];
    auto const& method = req["method"].getString();

    if (method == "Debugger.resume") {
      return folly::sformat("run -i {}", id);
    }
    throw Exception("Unknown command from chrome debugger client");
  } catch (const std::exception& exn) {
    throw Exception("XDebug: %s.  Command: '%s'.", exn.what(), chrome.data());
  }
}

//////////////////////////////////////////////////////////////////////

std::string dbgp_to_chrome(xdebug_xml_node* xml) {
  try {
    // XDebug sends this when it is starting up.
    if (strcmp(xml->tag, "init") == 0) {
      return "";
    }

    assert(strcmp(xml->tag, "response") == 0);

    auto const id_node = find_attr(xml, "transaction_id");
    auto const command_node = find_attr(xml, "command");

    // The end response has no command and no id.
    if (command_node == nullptr && id_node == nullptr) {
      // Verify this is the end response.
      if (strcmp(find_attr(xml, "status")->value, "stopping") == 0 &&
          strcmp(find_attr(xml, "reason")->value, "ok") == 0) {
        return "";
      }
      throw Exception("Bad response");
    }

    dynamic dyn = dynamic::object("response", dynamic::object);
    auto& response = dyn["response"];
    response["id"] = StringPiece(id_node->value, id_node->value_len);

    auto const command = StringPiece(
      command_node->value,
      command_node->value_len
    );

    if (command == "run") {
      response["error"] = dynamic::object;
    } else {
      throw Exception("Invalid dbgp response");
    }

    auto const opts = folly::json::serialization_opts{};
    return folly::json::serialize(dyn, opts).toStdString();
  } catch (const std::exception& exn) {
    xdebug_str xstr = { 0, 0, nullptr };
    xdebug_xml_return_node(xml, &xstr);
    auto exn2 = Exception("XDebug: %s.  Response: '%s'", exn.what(), xstr.d);
    xdebug_str_free(&xstr);
    throw exn2;
  }
}

//////////////////////////////////////////////////////////////////////
}
