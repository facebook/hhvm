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
#include "hphp/runtime/ext/wddx/ext_wddx.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/actrec-args.h"

namespace HPHP {

WddxPacket::WddxPacket(const Variant& comment, bool manualPacket, bool sVar) :
                       m_packetClosed(false),
                       m_manualPacketCreation(manualPacket) {
  m_packetString.append("<wddxPacket version='1.0'>");

  if (!comment.isNull() && !sVar) {
    m_packetString.append("<header><comment>");
    m_packetString.append(comment);
    m_packetString.append("</comment></header>");
  } else {
    m_packetString.append("<header/>");
  }

  m_packetString.append("<data>");

  if (m_manualPacketCreation) {
    m_packetString.append("<struct>");
  }
}

bool WddxPacket::add_var(const String& varName, bool hasVarTag) {
  VarEnv* v = g_context->getOrCreateVarEnv();
  if (!v) return false;
  Variant varVariant = *reinterpret_cast<Variant*>(v->lookup(varName.get()));
  return recursiveAddVar(varName, varVariant, hasVarTag);
}

String WddxPacket::packet_end() {
  if (!m_packetClosed) {
    if (m_manualPacketCreation) {
      m_packetString.append("</struct>");
    }
    m_packetString.append("</data></wddxPacket>");
  }
  m_packetClosed = true;
  return m_packetString.detach();
}

bool WddxPacket::serialize_value(const Variant& varVariant) {
  return recursiveAddVar(empty_string_ref, varVariant, false);
}

bool WddxPacket::recursiveAddVar(const String& varName,
                                 const Variant& varVariant,
                                 bool hasVarTag) {

  bool isArray = varVariant.isArray();
  bool isObject = varVariant.isObject();

  if (isArray || isObject) {
    if (hasVarTag) {
      m_packetString.append("<var name='");
      m_packetString.append(varName.data());
      m_packetString.append("'>");
    }

    Array varAsArray;
    Object varAsObject = varVariant.toObject();
    if (isArray) varAsArray = varVariant.toArray();
    if (isObject) varAsArray = varAsObject.toArray();

    int length = varAsArray.length();
    if (length > 0) {
      ArrayIter it = ArrayIter(varAsArray);
      if (it.first().isString()) isObject = true;
      if (isObject) {
        m_packetString.append("<struct>");
        if (!isArray) {
          m_packetString.append("<var name='php_class_name'><string>");
          m_packetString.append(varAsObject->getClassName());
          m_packetString.append("</string></var>");
        }
      } else {
        m_packetString.append("<array length='");
        m_packetString.append(std::to_string(length));
        m_packetString.append("'>");
      }
      for (ArrayIter it(varAsArray); it; ++it) {
        Variant key = it.first();
        Variant value = it.second();
        recursiveAddVar(key.toString(), value, isObject);
      }
      if (isObject) {
        m_packetString.append("</struct>");
      }
      else {
        m_packetString.append("</array>");
      }
    }
    else {
      //empty object
      if (isObject) {
        m_packetString.append("<struct>");
        if (!isArray) {
          m_packetString.append("<var name='php_class_name'><string>");
          m_packetString.append(varAsObject->getClassName());
          m_packetString.append("</string></var>");
        }
        m_packetString.append("</struct>");
      }
    }
    if (hasVarTag) {
      m_packetString.append("</var>");
    }
    return true;
  }

  String varType = getDataTypeString(varVariant.getType());
  if (!getWddxEncoded(varType, "", varName, false).empty()) {
    String varValue;
    if (varType.compare("boolean") == 0) {
      varValue = varVariant.toBoolean() ? "true" : "false";
    } else {
      varValue = StringUtil::HtmlEncode(varVariant.toString(),
                                        StringUtil::QuoteStyle::Double,
                                        "UTF-8", false, false).toCppString();
    }
    m_packetString.append(
      getWddxEncoded(varType, varValue, varName, hasVarTag));
    return true;
  }

  return false;
}

String WddxPacket::getWddxEncoded(const String& varType,
                                  const String& varValue,
                                  const String& varName,
                                  bool hasVarTag) {
  if (varType.compare("NULL") == 0) {
    return wrapValue("<null/>", "", "", varName, hasVarTag);
  }
  if (varType.compare("boolean") == 0) {
    return wrapValue("<boolean value='", "'/>", varValue, varName, hasVarTag);
  }
  if (varType.compare("integer") == 0 || varType.compare("double") == 0) {
    return wrapValue("<number>", "</number>", varValue, varName, hasVarTag);
  }
  if (varType.compare("string") == 0) {
    return wrapValue("<string>", "</string>", varValue, varName, hasVarTag);
  }
  return "";
}

String WddxPacket::wrapValue(const String& start,
                             const String& end,
                             const String& varValue,
                             const String& varName,
                             bool hasVarTag) {
  StringBuffer valueStr;

  if (hasVarTag) {
    valueStr.append("<var name='");
    valueStr.append(varName);
    valueStr.append("'>");
  }

  valueStr.append(start);
  valueStr.append(varValue);
  valueStr.append(end);

  if (hasVarTag) {
    valueStr.append("</var>");
  }

  return valueStr.detach();
}

//////////////////////////////////////////////////////////////////////////////
// helpers

void find_var_recursive(const TypedValue* tv,
                        const req::ptr<WddxPacket>& wddxPacket) {
  if (tvIsString(tv)) {
    String var_name = tvCastToString(tv);
    wddxPacket->add_var(var_name, true);
  }
  if (tv->m_type == KindOfArray) {
    for (ArrayIter iter(tv->m_data.parr); iter; ++iter) {
      find_var_recursive(iter.secondRef().asTypedValue(), wddxPacket);
    }
  }
}

static TypedValue* add_vars_helper(ActRec* ar) {
  int start_index = 1;
  Resource packet_id = getArg<KindOfResource>(ar, 0);
  auto wddxPacket = cast<WddxPacket>(packet_id);

  for (int i = start_index; i < ar->numArgs(); i++) {
    auto const tv = getArg(ar, i);
    find_var_recursive(tv, wddxPacket);
  }
  return arReturn(ar, true);
}

static TypedValue* serialize_vars_helper(ActRec* ar) {
  auto wddxPacket = req::make<WddxPacket>(empty_string_variant_ref,
                                             true, true);
  int start_index = 0;
  for (int i = start_index; i < ar->numArgs(); i++) {
    auto const tv = getArg(ar, i);
    find_var_recursive(tv, wddxPacket);
  }
  Variant packet = wddxPacket->packet_end();
  return arReturn(ar, std::move(packet));
}

//////////////////////////////////////////////////////////////////////////////
// functions

static TypedValue* HHVM_FN(wddx_add_vars)(ActRec* ar) {
  return add_vars_helper(ar);
}

static TypedValue* HHVM_FN(wddx_serialize_vars)(ActRec* ar) {
  return serialize_vars_helper(ar);
}

static String HHVM_FUNCTION(wddx_packet_end, const Resource& packet_id) {
  return cast<WddxPacket>(packet_id)->packet_end();
}

static Resource HHVM_FUNCTION(wddx_packet_start, const Variant& comment) {
  return Resource(req::make<WddxPacket>(comment, true, false));
}

static String HHVM_FUNCTION(wddx_serialize_value, const Variant& var,
                            const Variant& comment) {
  auto wddxPacket = req::make<WddxPacket>(comment, false, false);
  wddxPacket->serialize_value(var);
  return wddxPacket->packet_end();
}

//////////////////////////////////////////////////////////////////////////////

class wddxExtension final : public Extension {
 public:
  wddxExtension() : Extension("wddx") {}
  void moduleInit() override {
    HHVM_FE(wddx_add_vars);
    HHVM_FE(wddx_packet_end);
    HHVM_FE(wddx_packet_start);
    HHVM_FE(wddx_serialize_value);
    HHVM_FE(wddx_serialize_vars);
    loadSystemlib();
  }
} s_wddx_extension;

// Uncomment for non-bundled module
//HHVM_GET_MODULE(wddx);

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
