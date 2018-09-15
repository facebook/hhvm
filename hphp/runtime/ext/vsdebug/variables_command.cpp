/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"

#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <folly/Format.h>

using folly::format;

namespace HPHP {
namespace VSDEBUG {

const StaticString s_user("user");
const StaticString s_core("Core");

static bool isArrayObjectType(const std::string className) {
  // HH\Vector and HH\Map are special in that they are objects but their
  // children look like array indicies.
  return className == "HH\\Vector" || className == "HH\\Map";
};

VariablesCommand::VariablesCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message),
    m_objectId{0} {

  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  const int variablesReference = tryGetInt(args, "variablesReference", -1);
  if (variablesReference < 0) {
    throw DebuggerCommandException("Invalid variablesReference specified.");
  }

  m_objectId = variablesReference;
}

VariablesCommand::~VariablesCommand() {
}

request_id_t VariablesCommand::targetThreadId(DebuggerSession* session) {
  ServerObject* obj = session->getServerObject(m_objectId);
  if (obj == nullptr) {
    return 0;
  }

  if (obj->objectType() == ServerObjectType::Scope) {
    ScopeObject* scope = static_cast<ScopeObject*>(obj);
    return scope->m_requestId;
  } else if (obj->objectType() == ServerObjectType::Variable) {
    VariableObject* variable = static_cast<VariableObject*>(obj);
    return variable->m_requestId;
  } else if (obj->objectType() == ServerObjectType::SubScope) {
    VariableSubScope* subScope = static_cast<VariableSubScope*>(obj);
    return subScope->m_requestId;
  }

  throw DebuggerCommandException("Unexpected server object type");
}

const std::string VariablesCommand::getUcVariableName(
  folly::dynamic& var,
  const char* ucKey
) {
  // Convert name to UC to allow for case-insensitive sort, but
  // cache the result on the object so we don't do this every
  // iteration of the sort operation.
  const std::string& str = var["name"].getString();
  const std::string ucStr = tryGetString(var, ucKey, "");

  if (!ucStr.empty() || str.empty()) {
    return ucStr;
  }

  std::string upperString = std::string(str);
  for (char& c : upperString) {
    c = std::toupper(c);
  }

  var[ucKey] = upperString;
  return tryGetString(var, ucKey, "");
}

void VariablesCommand::sortVariablesInPlace(folly::dynamic& vars) {
  constexpr char* ucKey = "uc_name";
  assertx(vars.isArray());

  std::sort(
    vars.begin(),
    vars.end(),
    [](folly::dynamic& a, folly::dynamic& b) {
      const std::string& strA = getUcVariableName(a, ucKey);
      const std::string& strB = getUcVariableName(b, ucKey);
      return strA.compare(strB) < 0;
    }
  );

  // Finally, remove the cached uc_names from the objects.
  for (auto it = vars.begin(); it != vars.end(); it++) {
    try {
      it->erase(ucKey);
    } catch (std::out_of_range &e) {
    }
  }
}

bool VariablesCommand::executeImpl(
  DebuggerSession* session,
  folly::dynamic* responseMsg
) {
  folly::dynamic body = folly::dynamic::object;
  folly::dynamic variables = folly::dynamic::array;
  auto& args = tryGetObject(getMessage(), "arguments", s_emptyArgs);

  ServerObject* obj = session->getServerObject(m_objectId);
  if (obj != nullptr) {
    int start = tryGetInt(args, "start", 0);
    int count = tryGetInt(args, "count", -1);
    auto requestId = targetThreadId(session);

    if (obj->objectType() == ServerObjectType::Scope) {
      ScopeObject* scope = static_cast<ScopeObject*>(obj);
      addScopeVariables(session, targetThreadId(session), scope, &variables);
      sortVariablesInPlace(variables);

      // Client can ask for a subsequence of the variables.
      auto startIt = variables.begin() + start;
      if (startIt != variables.begin() || count > 0) {
        if (count <= 0) {
          count = variables.size();
        } else if (start + count >= variables.size()) {
          count = variables.size() - start;
        }

        auto endIt = variables.begin() + start + count;
        variables = folly::dynamic::array(startIt, endIt);
      }
    } else if (obj->objectType() == ServerObjectType::Variable) {
      VariableObject* variable = static_cast<VariableObject*>(obj);
      addComplexChildren(
        session,
        requestId,
        start,
        count,
        variable,
        &variables
      );
    } else if (obj->objectType() == ServerObjectType::SubScope) {
      VariableSubScope* subScope = static_cast<VariableSubScope*>(obj);
      addSubScopes(
        subScope,
        session,
        requestId,
        start,
        count,
        &variables
      );
    }
  }

  body["variables"] = variables;
  (*responseMsg)["body"] = body;

  // Completion of this command does not resume the target.
  return false;
}

int VariablesCommand::countScopeVariables(
  DebuggerSession* session,
  const ScopeObject* scope,
  request_id_t requestId
) {
  return addScopeVariables(session, requestId, scope, nullptr);
}

int VariablesCommand::addScopeVariables(
  DebuggerSession* session,
  request_id_t requestId,
  const ScopeObject* scope,
  folly::dynamic* vars
) {
  assertx(vars == nullptr || vars->isArray());

  switch (scope->m_scopeType) {
    case ScopeType::Locals:
      return addLocals(session, requestId, scope, vars);

    case ScopeType::ServerConstants: {
      Variant v;
      int count = addConstants(session, requestId, s_user, nullptr);
      addScopeSubSection(
        session,
        "User Defined Constants",
        "[" + std::to_string(count) + "]",
        "",
        nullptr,
        count,
        ClassPropsType::UserDefinedConstants,
        requestId,
        v,
        vars
      );

      count = addConstants(session, requestId, s_core, nullptr);
      addScopeSubSection(
        session,
        "System Defined Constants",
        "[" + std::to_string(count) + "]",
        "",
        nullptr,
        count,
        ClassPropsType::SystemDefinedConstants,
        requestId,
        v,
        vars
      );

      return 2;
    }
    case ScopeType::Superglobals:
      return addSuperglobalVariables(session, requestId, scope, vars);

    default:
      assertx(false);
  }

  return 0;
}

void VariablesCommand::addSubScopes(
  VariableSubScope* subScope,
  DebuggerSession* session,
  request_id_t requestId,
  int start,
  int count,
  folly::dynamic* variables
) {
  if (subScope->m_subScopeType == ClassPropsType::UserDefinedConstants) {
    addConstants(session, requestId, s_user, variables);
    sortVariablesInPlace(*variables);
  } else if (subScope->m_subScopeType ==
              ClassPropsType::SystemDefinedConstants) {
    addConstants(session, requestId, s_core, variables);
    sortVariablesInPlace(*variables);
  } else {
    const auto obj = subScope->m_variable.toObject().get();
    if (obj == nullptr) {
      throw DebuggerCommandException("Expected a server object.");
    }

    const auto cls = obj->getVMClass();
    if (cls == nullptr) {
      throw DebuggerCommandException("Expected an object class.");
    }

    switch (subScope->m_subScopeType) {
      case ClassPropsType::Constants: {
        // Add all the constants for the specified class to the result array
        addClassConstants(
          session,
          requestId,
          start,
          count,
          cls,
          subScope->m_variable,
          variables
        );
        break;
      }

      case ClassPropsType::StaticProps: {
        // Add all the static props for the specified class to the
        // result array.
        addClassStaticProps(
          session,
          requestId,
          start,
          count,
          cls,
          subScope->m_variable,
          variables
        );
        break;
      }

      case ClassPropsType::PrivateBaseProps: {
        addClassPrivateProps(
          session,
          requestId,
          start,
          count,
          subScope,
          subScope->m_variable,
          variables
        );
        break;
      }

      default:
        throw DebuggerCommandException("Unexpected sub scope type");
    }
  }
}

int VariablesCommand::addLocals(
  DebuggerSession* session,
  request_id_t requestId,
  const ScopeObject* scope,
  folly::dynamic* vars
) {
  assertx(scope->m_scopeType == ScopeType::Locals);

  VMRegAnchor regAnchor;
  int count = 0;

  const int frameDepth = scope->m_frameDepth;
  auto const fp = g_context->getFrameAtDepth(frameDepth);

  // If the frame at the specified depth has a $this, include it.
  if (fp != nullptr &&
      fp->func() != nullptr &&
      fp->func()->cls() != nullptr &&
      fp->hasThis()) {

    count++;
    if (vars != nullptr) {
      Variant this_(fp->getThis());
      vars->push_back(serializeVariable(session, requestId, "this", this_));
    }
  }

  // Add each variable, filtering out superglobals
  auto const locals = getDefinedVariables(fp);
  for (ArrayIter iter(locals); iter; ++iter) {
    const std::string name = iter.first().toString().toCppString();
    if (isSuperGlobal(name) || name == "this") {
      continue;
    }

    if (vars != nullptr) {
      vars->push_back(
        serializeVariable(session, requestId, name, iter.second())
      );
    }

    count++;
  }

  return count;
}

int VariablesCommand::getCachedValue(
  DebuggerSession* session,
  const int cacheKey,
  folly::dynamic* vars
) {
  int count;

  folly::dynamic* cachedResult = session->getCachedVariableObject(cacheKey);
  if (cachedResult != nullptr) {
    // Found a cached copy of this response as a folly::dynamic. Use that.
    count = cachedResult->size();
    if (vars != nullptr) {
      *vars = *cachedResult;
    }
    return count;
  }

  return -1;
}

int VariablesCommand::addConstants(
  DebuggerSession* session,
  request_id_t requestId,
  const StaticString& category,
  folly::dynamic* vars
) {
  int count = 0;

  int cacheKey = -1;
  if (category == s_core) {
    cacheKey = DebuggerSession::kCachedVariableKeyServerConsts;
  } else if (category == s_user) {
    cacheKey = DebuggerSession::kCachedVariableKeyUserConsts;
  }

  if (cacheKey != -1) {
    int cacheCount = getCachedValue(session, cacheKey, vars);
    if (cacheCount >= 0) {
      return cacheCount;
    }
  }

  const auto& constants = lookupDefinedConstants(true)[category].toArray();
  for (ArrayIter iter(constants); iter; ++iter) {
    const std::string name = iter.first().toString().toCppString();
    if (vars != nullptr) {
      vars->push_back(
        serializeVariable(session, requestId, name, iter.second(), true)
      );
    }

    count++;
  }

  if (vars != nullptr && cacheKey != -1) {
    // Cache the result since the same JSON array is to be requested by the
    // client for every stack frame for every request for this pause of the
    // target.
    session->setCachedVariableObject(cacheKey, *vars);
  }

  return count;
}

bool VariablesCommand::isSuperGlobal(const std::string& name) {
  static const hphp_string_set superGlobals = []() {
    hphp_string_set superGlobals;
    superGlobals.insert("_SERVER");
    superGlobals.insert("_GET");
    superGlobals.insert("_POST");
    superGlobals.insert("_COOKIE");
    superGlobals.insert("_FILES");
    superGlobals.insert("_ENV");
    superGlobals.insert("_REQUEST");
    superGlobals.insert("_SESSION");
    return superGlobals;
  }();
  return name == "GLOBALS" || superGlobals.count(name);
}

int VariablesCommand::addSuperglobalVariables(
  DebuggerSession* session,
  request_id_t requestId,
  const ScopeObject* scope,
  folly::dynamic* vars
) {
  assertx(scope->m_scopeType == ScopeType::Superglobals);

  int cacheCount = getCachedValue(
    session,
    DebuggerSession::kCachedVariableKeyServerGlobals,
    vars
  );

  if (cacheCount >= 0) {
    return cacheCount;
  }

  int count = 0;
  const Array globals = php_globals_as_array();
  for (ArrayIter iter(globals); iter; ++iter) {
    const std::string name = iter.first().toString().toCppString();
    if (!isSuperGlobal(name)) {
      continue;
    }

    if (vars != nullptr) {
      vars->push_back(
        serializeVariable(session, requestId, name, iter.second())
      );
    }

    count++;
  }

  if (vars != nullptr) {
    // Cache the result since the same JSON array is to be requested by the
    // client for every stack frame for every request for this pause of the
    // target.
    session->setCachedVariableObject(
      DebuggerSession::kCachedVariableKeyServerGlobals,
      *vars
    );
  }

  return count;
}

const std::string VariablesCommand::getPHPVarName(const std::string& name) {
  std::string variableName = (name[0] != '$' && name[0] != ':')
    ? std::string("$") + name
    : name;
  return variableName;
}

folly::dynamic VariablesCommand::serializeVariable(
  DebuggerSession* session,
  request_id_t requestId,
  const std::string& name,
  const Variant& variable,
  bool doNotModifyName, /* = false */
  folly::dynamic* presentationHint /* = nullptr */
) {
  folly::dynamic var = folly::dynamic::object;
  const std::string variableName = doNotModifyName ? name : getPHPVarName(name);

  var["name"] = variableName;
  var["value"] = getVariableValue(variable);
  var["type"] = getTypeName(variable);

  // If the variable is an array or object, register it as a server object and
  // indicate how many children it has.
  if (variable.isArray() || variable.isObject()) {
    unsigned int id = session->generateVariableId(
      requestId,
      const_cast<Variant&>(variable)
    );

    if (variable.isObject()) {
      var["namedVariables"] =
        addObjectChildren(session, requestId, -1, -1, variable, nullptr);
    } else if (variable.isDict() || variable.isKeyset()) {
      var["namedVariables"] = variable.toArray().size();
    } else {
      var["indexedVariables"] = variable.toArray().size();
    }

    var["variablesReference"] = id;
  }

  if (presentationHint != nullptr) {
    var["presentationHint"]  = *presentationHint;
  }

  return var;
}

const char* VariablesCommand::getTypeName(const Variant& variable) {
  switch (variable.getType()) {
    case KindOfUninit:
    case KindOfNull:
      return "null";

    case KindOfBoolean:
      return "bool";

    case KindOfInt64:
      return "int";

    case KindOfDouble:
      return "double";

    case KindOfPersistentString:
    case KindOfString:
      return "string";

    case KindOfResource:
      return "resource";

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray: {
      if (variable.isVecArray()) {
        return "vec";
      }

      if (variable.isDict()) {
        return "dict";
      }

      if (variable.isKeyset()) {
        return "keyset";
      }

      return "array";
    }

    case KindOfObject:
      return variable.toCObjRef()->getClassName().c_str();

    case KindOfRef:
      return "reference";

    default:
      VSDebugLogger::Log(
        VSDebugLogger::LogLevelError,
        "Unknown type %d for variable!",
        (int)variable.getType()
      );
      return "UNKNOWN TYPE";
  }
}

const std::string VariablesCommand::getVariableValue(const Variant& variable) {
  const DataType type = variable.getType();

  switch (type) {
    // For primitive / scalar values, just return a string representation of
    // the variable's value.
    case KindOfUninit:
    case KindOfNull:
      return "null";

    case KindOfBoolean:
      return variable.toBooleanVal() ? "true" : "false";

    case KindOfInt64:
      return std::to_string(variable.toInt64Val());

    case KindOfDouble: {
      // Convert double to string, but remove any trailing 0s after the
      // tenths position.
      std::string dblString = std::to_string(variable.toDoubleVal());
      dblString.erase(dblString.find_last_not_of('0') + 1, std::string::npos);
      if (dblString[dblString.size() - 1] == '.') {
        dblString += "0";
      }
      return dblString;
    }

    case KindOfPersistentString:
    case KindOfString:
      return variable.toCStrRef().toCppString();

    case KindOfResource: {
      auto res = variable.toResource();
      std::string resourceDesc = "resource id='";
      resourceDesc += std::to_string(res->getId());
      resourceDesc += "' type='";
      resourceDesc += res->o_getResourceName().data();
      resourceDesc += "'";
      return resourceDesc;
    }

    case KindOfPersistentVec:
    case KindOfVec:
      return format("vec[{}]", variable.toArray().size()).str();
    case KindOfPersistentArray:
    case KindOfArray: {
      return format("array[{}]", variable.toArray().size()).str();
    }

    case KindOfPersistentDict:
    case KindOfDict: {
      return format("dict[{}]", variable.toArray().size()).str();
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset: {
      return format("keyset[{}]", variable.toArray().size()).str();
    }

    case KindOfRef:
      // Note: PHP references are not supported in Hack.
      return "reference";

    case KindOfObject:
      return variable.toCObjRef()->getClassName().c_str();

    default:
      return "Unexpected variable type";
  }
}

int VariablesCommand::addComplexChildren(
  DebuggerSession* session,
  request_id_t requestId,
  int start,
  int count,
  VariableObject* variable,
  folly::dynamic* vars
) {
  Variant& var = variable->m_variable;

  if (var.isObject()) {
    int result = addObjectChildren(
      session,
      requestId,
      start,
      count,
      variable->m_variable,
      vars
    );

    bool isArrayLikeObject = false;
    const auto obj = var.toObject().get();
    if (obj != nullptr) {
      auto currentClass = obj->getVMClass();
      if (currentClass != nullptr) {
        const std::string className = currentClass->name()->toCppString();
        isArrayLikeObject = isArrayObjectType(className);
      }
    }

    if (vars != nullptr && !isArrayLikeObject) {
      sortVariablesInPlace(*vars);
    }

    // NOTE: These are added after the instance variables are added and sorted
    // so that they appear at the end of the scope block in the UX.

    // Constants defined on this object's class and all constants inherited
    // from classes up the parent chain.
    result += addClassSubScopes(
      session,
      ClassPropsType::Constants,
      requestId,
      var,
      vars
    );

    // Static props defined on this object's class and all static props
    // inherited from classes up the parent chain.
    result += addClassSubScopes(
      session,
      ClassPropsType::StaticProps,
      requestId,
      var,
      vars
    );

    return result;
  } else if (var.isArray()) {
    return addArrayChildren(session, requestId, start, count, variable, vars);
  }

  return 0;
}

int VariablesCommand::addArrayChildren(
  DebuggerSession* session,
  request_id_t requestId,
  int start,
  int count,
  VariableObject* variable,
  folly::dynamic* vars
) {
  Variant& var = variable->m_variable;

  assertx(var.isArray());

  int idx = -1;
  int added = 0;

  for (ArrayIter iter(var.toArray()); iter; ++iter) {
    idx++;

    if (start >= 0 && idx < start) {
      continue;
    }

    std::string name = iter.first().toString().toCppString();
    if (!iter.first().isInteger()) {
      name = "'" + name + "'";
    }

    if (vars != nullptr) {
      vars->push_back(
        serializeVariable(session, requestId, name, iter.second(), true)
      );
      added++;
    }

    if (count > 0 && added >= count) {
      break;
    }
  }

  return added;
}

int VariablesCommand::addClassConstants(DebuggerSession* session,
                                        request_id_t requestId, int start,
                                        int count, Class* cls,
                                        const Variant& /*var*/,
                                        folly::dynamic* vars) {
  Class* currentClass = cls;

  std::unordered_set<std::string> constantNames;
  while (currentClass != nullptr) {
    for (Slot i = 0; i < currentClass->numConstants(); i++) {
      auto& constant = currentClass->constants()[i];

      folly::dynamic presentationHint = folly::dynamic::object;
      folly::dynamic attribs = folly::dynamic::array;
      attribs.push_back("constant");
      attribs.push_back("readOnly");
      presentationHint["attributes"] = attribs;

      std::string name = constant.cls->name()->toCppString() +
          "::" +
          constant.name->toCppString();

      if (constantNames.find(name) == constantNames.end()) {
        constantNames.insert(name);
        if (vars != nullptr) {
          vars->push_back(
            serializeVariable(
              session,
              requestId,
              name,
              tvAsVariant(const_cast<TypedValueAux*>(&constant.val)),
              true,
              &presentationHint
            )
          );
        }
      }
    }

    currentClass = currentClass->parent();
  }

  if (vars != nullptr) {
    sortVariablesInPlace(*vars);

    if (start > 0 && start < vars->size()) {
      vars->erase(vars->begin(), vars->begin() + start);
    }

    if (count > 0) {
      if (count > vars->size()) {
        count = vars->size();
      }
      vars->erase(vars->begin() + count, vars->end());
    }
  }

  return constantNames.size();
}

int VariablesCommand::addClassStaticProps(DebuggerSession* session,
                                          request_id_t requestId, int start,
                                          int count, Class* cls,
                                          const Variant& /*var*/,
                                          folly::dynamic* vars) {
  const std::string className = cls->name()->toCppString();
  const auto staticProperties = cls->staticProperties();
  int propCount = 0;

  for (Slot i = 0; i < cls->numStaticProperties(); i++) {
    const auto& prop = staticProperties[i];
    auto val = cls->getSPropData(i);

    if (val != nullptr) {
      Variant variant = tvAsVariant(val);

      folly::dynamic presentationHint = folly::dynamic::object;
      folly::dynamic attribs = folly::dynamic::array;
      attribs.push_back("static");

      if (prop.attrs & AttrInterface) {
        attribs.push_back("interface");
      }

      if (prop.attrs & AttrPublic) {
        presentationHint["visibility"] = "public";
      } else if (prop.attrs & AttrProtected) {
        presentationHint["visibility"] = "protected";
      } else if (prop.attrs & AttrPrivate) {
        presentationHint["visibility"] = "private";
      }

      if (variant.isObject()) {
        presentationHint["kind"] = "class";
      }

      presentationHint["attributes"] = attribs;
      std::string propName =
        className + "::$" + prop.name.get()->toCppString();

      propCount++;

      if (vars != nullptr) {
        vars->push_back(
          serializeVariable(
            session,
            requestId,
            propName,
            variant,
            true,
            &presentationHint
          )
        );
      }
    }
  }

  if (vars != nullptr) {
    sortVariablesInPlace(*vars);

    if (start > 0 && start < vars->size()) {
      vars->erase(vars->begin(), vars->begin() + start);
    }

    if (count > 0) {
      if (count > vars->size()) {
        count = vars->size();
      }
      vars->erase(vars->begin() + count, vars->end());
    }
  }

  return propCount;
}

int VariablesCommand::addScopeSubSection(
  DebuggerSession* session,
  const char* displayName,
  const std::string& displayValue,
  const std::string& className,
  const Class* currentClass,
  int childCount,
  ClassPropsType type,
  request_id_t requestId,
  const Variant& var,
  folly::dynamic* vars
) {
  int constantScopeId =
    session->generateVariableSubScope(
      requestId,
      var,
      currentClass,
      className,
      type
    );

  if (vars != nullptr) {
    folly::dynamic container = folly::dynamic::object;
    folly::dynamic presentationHint = folly::dynamic::object;
    folly::dynamic attribs = folly::dynamic::array;
    attribs.push_back("constant");
    attribs.push_back("readOnly");
    presentationHint["attributes"] = attribs;

    container["name"] = displayName;
    container["value"] = displayValue;
    container["namedVariables"] = childCount;
    container["presentationHint"] = presentationHint;
    container["variablesReference"] = constantScopeId;
    vars->push_back(container);
  }

  return constantScopeId;
}

int VariablesCommand::addClassSubScopes(
  DebuggerSession* session,
  ClassPropsType propType,
  request_id_t requestId,
  const Variant& var,
  folly::dynamic* vars
) {
  int subScopeCount = 0;
  const auto obj = var.toObject().get();
  if (obj == nullptr) {
    return 0;
  }

  auto currentClass = obj->getVMClass();
  if (currentClass == nullptr) {
    return 0;
  }

  int count = 0;
  const char* scopeTitle = nullptr;
  std::string className = currentClass->name()->toCppString();

  switch (propType) {
    case ClassPropsType::Constants: {
        count = addClassConstants(
          session,
          requestId,
          0,
          0,
          currentClass,
          var,
          nullptr
        );
        scopeTitle = "Class Constants";
        if (count > 0) {
          subScopeCount++;
          addScopeSubSection(
            session,
            scopeTitle,
            "class " + className,
            className,
            currentClass,
            count,
            propType,
            requestId,
            var,
            vars
          );
        }
      }
      break;
    case ClassPropsType::StaticProps: {
        className = currentClass->name()->toCppString();
        count = addClassStaticProps(
          session,
          requestId,
          0,
          0,
          currentClass,
          var,
          nullptr
        );
        scopeTitle = "Static Props";
        if (count > 0) {
          subScopeCount++;
          addScopeSubSection(
            session,
            scopeTitle,
            "class " + className,
            className,
            currentClass,
            count,
            propType,
            requestId,
            var,
            vars
          );
        }
      }
      break;
    default:
      assertx(false);
  }

  return subScopeCount;
}

void VariablesCommand::forEachInstanceProp(
  const Variant& var,
  std::function<bool(
    const std::string& objectClassName,
    const std::string& propName,
    const std::string& propClassName,
    const std::string& displayName,
    const char* visibilityDescription,
    folly::dynamic& presentationHint,
    const Variant& propertyVariant
  )> callback
) {
  const auto obj = var.toObject().get();
  if (obj == nullptr) {
    return;
  }

  const auto cls = obj->getVMClass();
  if (cls == nullptr) {
    return;
  }

  // Instance properties on this object.
  const Array instProps = obj->toArray(false, true);
  const std::string className = cls->name()->toCppString();

  for (ArrayIter iter(instProps); iter; ++iter) {
    std::string propName = iter.first().toString().toCppString();
    const Variant& propertyVariant = iter.second();

    std::string propClassName = className;
    const char* visibilityDescription;

    // The object's property name can be encoded with info about the modifier
    // and class. Decode it. (See HPHP::PreClass::manglePropName).
    if (propName.size() < 3 || propName[0] != '\0') {
      // This is a public property.
      visibilityDescription = VisibilityPublic;
    } else if (propName[1] == '*') {
      // This is a protected property.
      propName = propName.substr(3);
      visibilityDescription = VisibilityProtected;
    } else {
      // This is a private property on this object class or one of its base
      // classes.
      visibilityDescription = VisibilityPrivate;
      const unsigned long classNameEnd = propName.find('\0', 1);
      propClassName = propName.substr(1, classNameEnd - 1);
      propName = propName.substr(classNameEnd + 1);
    }

    const std::string displayName = isArrayObjectType(className)
      ? propName
      : "$" + propName;

    folly::dynamic presentationHint = folly::dynamic::object;
    presentationHint["visibility"] = visibilityDescription;

    if (propertyVariant.isObject()) {
      presentationHint["kind"] = "class";
    }

    bool continueLooping = callback(
                             className,
                             propName,
                             propClassName,
                             displayName,
                             visibilityDescription,
                             presentationHint,
                             propertyVariant
                           );

    if (!continueLooping) {
      break;
    }
  }
}

void VariablesCommand::addClassPrivateProps(
  DebuggerSession* session,
  request_id_t requestId,
  int start,
  int count,
  VariableSubScope* subScope,
  const Variant& var,
  folly::dynamic* vars
) {
  int propCount = 0;

  forEachInstanceProp(
    var,
    [&](
      const std::string& objectClassName,
      const std::string& propName,
      const std::string& propClassName,
      const std::string& displayName,
      const char* visibilityDescription,
      folly::dynamic& presentationHint,
      const Variant& propertyVariant
    ) {

      // Only looking for private properties.
      if (visibilityDescription != VisibilityPrivate) {
        return true;
      }

      // And only private properties on the specified class.
      if (propClassName != subScope->m_className) {
        return true;
      }

      propCount++;
      if (start >= 0 && propCount < start) {
        return true;
      }

      vars->push_back(
        serializeVariable(
          session,
          requestId,
          displayName,
          propertyVariant,
          true,
          &presentationHint
        )
      );

      if (count > 0 && propCount - start >= count) {
        return false;
      }

      return true;
  });
}

int VariablesCommand::addObjectChildren(
  DebuggerSession* session,
  request_id_t requestId,
  int start,
  int count,
  const Variant& var,
  folly::dynamic* vars
) {
  assertx(var.isObject());

  int propCount = 0;

  // The inheritance rules for public and protected properties are such that
  // the first property with a given name encountered when walking up from
  // the most-derived class type to the base-most class type, shadows all
  // other instances, and is the only one that actually exists on the object,
  // from any context.
  std::unordered_set<std::string> properties;
  std::unordered_map<std::string, int> privPropScopes;

  forEachInstanceProp(
    var,
    [&](
      const std::string& objectClassName,
      const std::string& propName,
      const std::string& propClassName,
      const std::string& displayName,
      const char* visibilityDescription,
      folly::dynamic& presentationHint,
      const Variant& propertyVariant
    ) {
      if (vars == nullptr) {
        // We are only counting.
        propCount++;
        return true;
      }

      if (propClassName != objectClassName) {
        // If this is a private member variable and the variable's declaring
        // class name is not the same as the class name of the current object,
        // this is a private member declared in a parent class. In this case,
        // the private member is not accessible from code in the derived
        // classes, but the property is not shadowed on the PHP object: it is
        // visible and accessible from methods on the base class.
        auto it = privPropScopes.find(propClassName);
        if (it == privPropScopes.end()) {
          // If this is the first private property encountered for this
          // particular parent class, add a scope section for that class's
          // private properties.
          privPropScopes.emplace(propClassName, 1);
        } else {
          it->second++;
        }

        // Don't add the private property to this scope, add it to a child
        // scope below.
        return true;
      }

      if (properties.find(displayName) != properties.end()) {
        // A property with this name has already been added by a derived class,
        // since we're walking the class inheritance hierarchy from the object
        // upwards, the derived property shadows this property.
        return true;
      }

      properties.insert(displayName);
      propCount++;

      if (start >= 0 && propCount < start) {
        return true;
      }

      vars->push_back(
        serializeVariable(
          session,
          requestId,
          displayName,
          propertyVariant,
          true,
          &presentationHint
        )
      );

      if (count > 0 && propCount - start >= count) {
        return false;
      }

      return true;
    });

  if (vars == nullptr) {
    return propCount;
  }

  // Create a scope for each base class that had shadowed private properties.
  for (auto it = privPropScopes.begin(); it != privPropScopes.end(); it++) {
    const std::string& propClassName = it->first;
    int privPropCount = it->second;
    addScopeSubSection(
      session,
      "Private props",
      "class " + propClassName,
      propClassName,
      nullptr,
      privPropCount,
      ClassPropsType::PrivateBaseProps,
      requestId,
      var,
      vars
    );
  }

  return propCount;
}

}
}
