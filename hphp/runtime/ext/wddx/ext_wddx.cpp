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

#include <string>
#include <vector>

namespace HPHP {

WddxPacket::WddxPacket(const Variant& comment, bool manualPacket, bool sVar) :
                       m_packetString(""), m_packetClosed(false),
                       m_manualPacketCreation(manualPacket) {
  std::string header = "<header/>";

  if (!comment.isNull() && !sVar) {
    std::string scomment = comment.toString().data();
    header = "<header><comment>" + scomment + "</comment></header>";
  }
  m_packetString = "<wddxPacket version='1.0'>" + header + "<data>";

  if (m_manualPacketCreation) {
    m_packetString = m_packetString + "<struct>";
  }
}

bool WddxPacket::add_var(const String& varName, bool hasVarTag) {
  VarEnv* v = g_context->getVarEnv();
  if (!v) return false;
  Variant varVariant = *reinterpret_cast<Variant*>(v->lookup(varName.get()));
  return recursiveAddVar(varName, varVariant, hasVarTag);
}

std::string WddxPacket::packet_end() {
  if (!m_packetClosed) {
    if (m_manualPacketCreation) {
      m_packetString += "</struct>";
    }
    m_packetString += "</data></wddxPacket>";
  }
  m_packetClosed = true;
  return m_packetString;
}

bool WddxPacket::serialize_value(const Variant& varVariant) {
  return recursiveAddVar(empty_string, varVariant, false);
}

bool WddxPacket::recursiveAddVar(String varName, const Variant& varVariant,
                                 bool hasVarTag) {

  bool isArray = varVariant.isArray();
  bool isObject = varVariant.isObject();

  if (isArray || isObject) {
    if (hasVarTag) {
      m_packetString += "<var name='";
      m_packetString += varName.data();
      m_packetString += "'>";
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
        m_packetString += "<struct>";
        if (!isArray) {
          m_packetString += "<var name='php_class_name'><string>";
          m_packetString += varAsObject->o_getClassName().c_str();
          m_packetString += "</string></var>";
        }
      } else {
        m_packetString += "<array length='";
        m_packetString += std::to_string(length);
        m_packetString += "'>";
      }
      for (ArrayIter it(varAsArray); it; ++it) {
        Variant key = it.first();
        Variant value = it.second();
        recursiveAddVar(key.toString(), value, isObject);
      }
      if (isObject) {
        m_packetString += "</struct>";
      }
      else {
        m_packetString += "</array>";
      }
    }
    else {
      //empty object
      if (isObject) {
        m_packetString += "<struct>";
        if (!isArray) {
          m_packetString += "<var name='php_class_name'><string>";
          m_packetString += varAsObject->o_getClassName().c_str();
          m_packetString += "</string></var>";
        }
        m_packetString += "</struct>";
      }
    }
    if (hasVarTag) {
      m_packetString += "</var>";
    }
    return true;
  }

  std::string varType = getDataTypeString(varVariant.getType()).data();
  if (!getWddxEncoded(varType, "", varName, false).empty()) {
    std::string varValue = varVariant.toString().data();
    if (varType.compare("boolean") == 0) {
      varValue = varVariant.toBoolean() ? "true" : "false";
    }
    m_packetString += getWddxEncoded(varType, varValue, varName, hasVarTag);
    return true;
  }

  return false;
}

std::string WddxPacket::getWddxEncoded(std::string varType,
                                       std::string varValue,
                                       String varName,
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

std::string WddxPacket::wrapValue(std::string start,
                                  std::string end,
                                  std::string varValue,
                                  String varName,
                                  bool hasVarTag) {
  std::string startVar = "";
  std::string endVar = "";
  if (hasVarTag) {
    startVar += "<var name='";
    startVar += varName.data();
    startVar += "'>";
    endVar = "</var>";
  }
  return startVar + start + varValue + end + endVar;
}

//////////////////////////////////////////////////////////////////////////////
// helpers

void find_var_recursive(const TypedValue* tv, WddxPacket* wddxPacket) {
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
  auto wddxPacket = packet_id.getTyped<WddxPacket>();

  for (int i = start_index; i < ar->numArgs(); i++) {
    auto const tv = getArg(ar, i);
    find_var_recursive(tv, wddxPacket);
  }
  return arReturn(ar, true);
}

static TypedValue* serialize_vars_helper(ActRec* ar) {
  WddxPacket* wddxPacket = NEWOBJ(WddxPacket)(empty_string, true, true);
  int start_index = 0;
  for (int i = start_index; i < ar->numArgs(); i++) {
    auto const tv = getArg(ar, i);
    find_var_recursive(tv, wddxPacket);
  }
  const std::string packet = wddxPacket->packet_end();
  Variant strHolder = makeStaticString(packet);
  return arReturn(ar, strHolder);
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
  auto wddxPacket = packet_id.getTyped<WddxPacket>();
  std::string packetString = wddxPacket->packet_end();
  return String(packetString);
}

static Resource HHVM_FUNCTION(wddx_packet_start, const Variant& comment) {
  auto wddxPacket = NEWOBJ(WddxPacket)(comment, true, false);
  return Resource(wddxPacket);
}

static String HHVM_FUNCTION(wddx_serialize_value, const Variant& var,
                            const Variant& comment) {
  WddxPacket* wddxPacket = NEWOBJ(WddxPacket)(comment, false, false);
  wddxPacket->serialize_value(var);
  const std::string packetString = wddxPacket->packet_end();
  return String(packetString);
}

//////////////////////////////////////////////////////////////////////////////

class wddxExtension : public Extension {
 public:
  wddxExtension() : Extension("wddx") {}
  virtual void moduleInit() {
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
