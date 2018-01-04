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

#include "hphp/runtime/base/backtrace.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {
namespace VSDEBUG {

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

int64_t VariablesCommand::targetThreadId(DebuggerSession* session) {
  ServerObject* obj = session->getServerObject(m_objectId);
  if (obj == nullptr) {
    throw DebuggerCommandException("Invalid variablesReference specified.");
  }

  if (obj->objectType() == ServerObjectType::Scope) {
    ScopeObject* scope = static_cast<ScopeObject*>(obj);
    return scope->m_requestId;
  } else if (obj->objectType() == ServerObjectType::Variable) {
    VariableObject* variable = static_cast<VariableObject*>(obj);
    return variable->m_requestId;
  }

  throw DebuggerCommandException("Unexpected server object type");
}

const std::string& VariablesCommand::getUcVariableName(
  folly::dynamic& var,
  const char* ucKey
) {
  // Convert name to UC to allow for case-insensitive sort, but
  // cache the result on the object so we don't do this every
  // iteration of the sort operation.
  const std::string& str = var["name"].getString();
  const std::string& ucStr = tryGetString(var, ucKey, "");

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
  assert(vars.isArray());

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
    } catch (std::out_of_range e) {
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
  if (obj == nullptr) {
    throw DebuggerCommandException("Invalid variablesReference specified.");
  }

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
  }

  body["variables"] = variables;
  (*responseMsg)["body"] = body;

  // Completion of this command does not resume the target.
  return false;
}

const StaticString s_user("user");
const StaticString s_core("Core");

int VariablesCommand::countScopeVariables(const ScopeObject* scope) {
  return addScopeVariables(nullptr, -1, scope, nullptr);
}

int VariablesCommand::addScopeVariables(
  DebuggerSession* session,
  int64_t requestId,
  const ScopeObject* scope,
  folly::dynamic* vars
) {
  assert(vars == nullptr || vars->isArray());

  switch (scope->m_scopeType) {
    case Locals:
      return addLocals(session, requestId, scope, vars);

    case UserDefinedConstants:
      return addConstants(session, requestId, scope, s_user, vars);

    case CoreConstants:
      return addConstants(session, requestId, scope, s_core, vars);

    case Superglobals:
      return addSuperglobalVariables(session, requestId, scope, vars);

    default:
      assert(false);
  }

  return 0;
}

int VariablesCommand::addLocals(
  DebuggerSession* session,
  int64_t requestId,
  const ScopeObject* scope,
  folly::dynamic* vars
) {
  assert(scope->m_scopeType == ScopeType::Locals);

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
    if (isSuperGlobal(name)) {
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

int VariablesCommand::addConstants(
  DebuggerSession* session,
  int64_t requestId,
  const ScopeObject* scope,
  const StaticString& category,
  folly::dynamic* vars
) {
  assert(scope->m_scopeType == ScopeType::UserDefinedConstants ||
         scope->m_scopeType == ScopeType::CoreConstants);

  int count = 0;

  const auto& constants = lookupDefinedConstants(true)[category].toArray();
  for (ArrayIter iter(constants); iter; ++iter) {
    const std::string name = iter.first().toString().toCppString();
    if (vars != nullptr) {
      vars->push_back(
        serializeVariable(session, requestId, name, iter.second())
      );
    }

    count++;
  }

  return count;
}

bool VariablesCommand::isSuperGlobal(const std::string& name) {
  return name == "GLOBALS" || BuiltinSymbols::IsSuperGlobal(name);
}

int VariablesCommand::addSuperglobalVariables(
  DebuggerSession* session,
  int64_t requestId,
  const ScopeObject* scope,
  folly::dynamic* vars
) {
  assert(scope->m_scopeType == ScopeType::Superglobals);

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
  int64_t requestId,
  const std::string& name,
  const Variant& variable,
  bool childProp /* = false */
) {
  folly::dynamic var = folly::dynamic::object;
  const std::string variableName = childProp ? name : getPHPVarName(name);

  var["name"] = variableName;
  var["value"] = getVariableValue(variable);
  var["type"] = getTypeName(variable);

  // If the variable is an array, register it as a server object and indicate
  // how many indicies it has.
  if (variable.isArray()) {
    unsigned int id = session->generateVariableId(
      requestId,
      const_cast<Variant&>(variable)
    );

    if (variable.isDict() || variable.isKeyset()) {
      var["namedVariables"] = variable.toArray().size();
    } else {
      var["indexedVariables"] = variable.toArray().size();
    }

    var["variablesReference"] = id;
  }

  //var["presentationHint"]  = ...  TODO
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
      return "undefined";

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
    case KindOfPersistentArray:
    case KindOfArray: {
      std::string arrayDesc = "array[";
      arrayDesc += std::to_string(variable.toArray().size());
      arrayDesc += "]";
      return arrayDesc;
    }

    case KindOfPersistentDict:
    case KindOfDict: {
      std::string dictDisc = "dict[";
      dictDisc += std::to_string(variable.toArray().size());
      dictDisc += "]";
      return dictDisc;
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset: {
      std::string keysetDisc = "keyset[";
      keysetDisc += std::to_string(variable.toArray().size());
      keysetDisc += "]";
      return keysetDisc;
    }

    case KindOfRef:
      // Note: PHP references are not supported in Hack.
      return "reference";

    // TODO: Implement serialization for complex types here.
    default: return "TODO";
  }
}

int VariablesCommand::addComplexChildren(
  DebuggerSession* session,
  int64_t requestId,
  int start,
  int count,
  VariableObject* variable,
  folly::dynamic* vars
) {
  Variant& var = variable->m_variable;

  if (var.isArray()) {
    return addArrayChildren(session, requestId, start, count, variable, vars);
  }

  return 0;
}

int VariablesCommand::addArrayChildren(
  DebuggerSession* session,
  int64_t requestId,
  int start,
  int count,
  VariableObject* variable,
  folly::dynamic* vars
) {
  Variant& var = variable->m_variable;

  assert(var.isArray());

  int idx = 0;
  int added = 0;

  for (ArrayIter iter(var.toArray()); iter; ++iter) {
    if (start >= 0 && idx < start) {
      continue;
    }

    const std::string name = iter.first().toString().toCppString();
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

}
}
