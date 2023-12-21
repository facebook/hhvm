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
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/debugger-request-info.h"
#include "hphp/runtime/ext/vsdebug/php_executor.h"
#include "hphp/runtime/vm/runtime-compiler.h"
#include "hphp/util/timer.h"

namespace HPHP {
namespace VSDEBUG {

namespace {
struct EvaluatePHPExecutor: public PHPExecutor
{
public:
  EvaluatePHPExecutor(
    Debugger *debugger,
    DebuggerSession *session,
    request_id_t m_threadId,
    const std::string &expr,
    int frameDepth,
    bool evalSilent
  );

  ExecutionContext::EvaluationResult m_result;

protected:
  std::string m_expr;
  Unit *m_rawUnit;
  int m_frameDepth;

  void callPHPCode() override;
};
}

EvaluatePHPExecutor::EvaluatePHPExecutor(
  Debugger *debugger,
  DebuggerSession *session,
  request_id_t threadId,
  const std::string &expr,
  int frameDepth,
  bool evalSilent
) : PHPExecutor(debugger, session, "Evaluation returned", threadId, evalSilent)
  , m_expr{expr}
  , m_frameDepth{frameDepth}
{
}

void EvaluatePHPExecutor::callPHPCode()
{
  std::unique_ptr<Unit> unit(compile_debugger_string(m_expr.c_str(),
                              m_expr.size(),
                              g_context->getRepoOptionsForFrame(m_frameDepth)));
  if (!unit) {
    // The compiler will already have printed more detailed error messages
    // to stderr, which is redirected to the debugger client's console.
    throw DebuggerCommandException("Error compiling expression.");
  }

  Unit* rawUnit = unit.get();
  m_ri->m_evaluationUnits.push_back(std::move(unit));
  if (m_evalSilent) {
    SilentEvaluationContext silentContext(m_debugger, m_ri);
    m_result = g_context->evalPHPDebugger(rawUnit, m_frameDepth);
  } else {
    m_result = g_context->evalPHPDebugger(rawUnit, m_frameDepth);
    if (!m_result.error.empty()) {
      m_debugger->sendUserMessage(
        m_result.error.c_str(),
        DebugTransport::OutputLevelError
      );
    }
  }
}

EvaluateCommand::EvaluateCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message),
    m_frameId{0},
    m_returnHhvmSerialization{false} {

  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  const int frameId = tryGetInt(args, "frameId", -1);
  const bool returnHhvmSerialization = tryGetBool(
    args, "returnHhvmSerialization", false);
  m_frameId = frameId;
  m_returnHhvmSerialization = returnHhvmSerialization;
}

EvaluateCommand::~EvaluateCommand() {
}

FrameObject* EvaluateCommand::getFrameObject(DebuggerSession* session) {
  if (m_frameObj != nullptr) {
    return m_frameObj;
  }

  auto currFrameId = (m_frameId < 0 && m_debugger->isPaused()) ?
    session->getCurrFrameId() : m_frameId;

  m_frameObj = session->getFrameObject(currFrameId);
  return m_frameObj;
}

request_id_t EvaluateCommand::targetThreadId(DebuggerSession* session) {
  FrameObject* frame = getFrameObject(session);
  if (frame == nullptr) {
    // Execute the eval in the dummy context.
    return Debugger::kDummyTheadId;
  }

  return frame->m_requestId;
}

void EvaluateCommand::logToScuba(const std::string& code,
                                 bool success,
                                 const std::string& error,
                                 const std::string& clientId,
                                 uint32_t sessionId,
                                 int64_t before,
                                 int64_t after,
                                 bool bpHit) {
  StructuredLogEntry ent;
  ent.setStr("code", code);
  ent.setInt("num_chars", code.size());
  auto num_lines = std::count(code.begin(), code.end(), '\n');
  num_lines += (!code.empty() && code.back() != '\n');
  ent.setInt("num_lines", num_lines);
  ent.setInt("success", success);
  ent.setStr("error", error);
  ent.setStr("client_id", clientId);
  ent.setInt("session_id", sessionId);
  ent.setInt("start_time", before);
  ent.setInt("end_time", after);
  ent.setInt("bp_hit", bpHit);
  StructuredLog::log("hphp_debugger_repl_logs", ent);
}

static const StaticString s_varName("_");

bool EvaluateCommand::executeImpl(
  DebuggerSession* session,
  folly::dynamic* responseMsg
) {
  folly::dynamic& message = getMessage();
  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  const auto threadId = targetThreadId(session);

  if (m_frameId < 0 && threadId != m_debugger->getCurrentThreadId()) {
    throw DebuggerCommandException("Evaluate command running in wrong context");
  }

  auto const rawExpression = tryGetString(args, "expression", "");
  auto const evalExpression = prepareEvalExpression(rawExpression);

  FrameObject* frameObj = getFrameObject(session);
  int frameDepth = frameObj == nullptr ? 0 : frameObj->m_frameDepth;

  const std::string evalContext = tryGetString(args, "context", "");
  bool evalSilent = evalContext == "watch" || evalContext == "hover";

  auto executor = EvaluatePHPExecutor{
    m_debugger,
    session,
    threadId,
    evalExpression,
    frameDepth,
    evalSilent
  };

  auto isDummy = m_debugger->isDummyRequest();
  DebuggerRequestInfo *dummyRI = nullptr;
  if (isDummy) {
    dummyRI = m_debugger->getRequestInfo();
    m_debugger->checkForFileChanges(dummyRI);
    dummyRI->m_firstBpHit = false;
  }
  auto before = gettime_ns(CLOCK_REALTIME);
  executor.execute();
  auto after = gettime_ns(CLOCK_REALTIME);

  auto result = executor.m_result;

  if (isDummy &&
      RuntimeOption::LogEvaluationCommands &&
      StructuredLog::enabled()) {
    logToScuba(rawExpression, !result.failed, result.error,
               session->getClientId(), session->getSessionId(),
               before, after, dummyRI->m_firstBpHit);
  }

  if (result.failed) {
    if (!evalSilent) {
      // Note that the VM will have already sent the message to stderr
      throw DebuggerCommandException("Failed to evaluate expression");
    } else {
      // Return empty response with no type in silent eval context.
      (*responseMsg)["body"] = folly::dynamic::object;
      folly::dynamic& body = (*responseMsg)["body"];
      body["type"] = "";
      return false;
    }
  }

  if (evalContext == "repl") {
    // Note that the execution code, if it succeeded, should have created
    // a varenv at the frame already.
    auto& denv = g_context->getDebuggerEnv();
    if (denv.isNull()) denv = Array::CreateDict();
    denv.set(StrNR{s_varName.get()}, executor.m_result.result, true);
  }

  if (!m_debugger->getDebuggerOptions().disablePostDummyEvalHelper &&
      m_debugger->isDummyRequest() &&
      m_debugger->getRequestInfo()->m_evaluateCommandDepth == 0) {
    auto helperExecutor = EvaluatePHPExecutor{
      m_debugger,
      session,
      threadId,
      "<?hh if (\\function_exists('vsdebug_post_dummy_eval')) { \\vsdebug_post_dummy_eval(); }",
      frameDepth,
      false
    };
    helperExecutor.execute();
    if (helperExecutor.m_result.failed) {
      VSDebugLogger::Log(
        VSDebugLogger::LogLevelError,
        "Failed to evaluate vsdebug_post_dummy_eval"
      );
    }
  }

  folly::dynamic serializedResult =
    VariablesCommand::serializeVariable(
      session,
      m_debugger,
      threadId,
      "",
      result.result
    );

  (*responseMsg)["body"] = folly::dynamic::object;
  folly::dynamic& body = (*responseMsg)["body"];

  if (m_returnHhvmSerialization) {
    try {
      VariableSerializer vs(
        VariableSerializer::Type::DebuggerDump,
        0,
        2
      );
      body["serialized"] = vs.serialize(result.result, true).get()->data();
    } catch (const StringBufferLimitException& ) {
      body["serialized"] = "Serialization limit exceeded";
    } catch (...) {
      assertx(false);
      throw;
    }
  }

  body["result"] = serializedResult["value"];
  body["type"] = serializedResult["type"];

  int variableReference = tryGetInt(serializedResult, "variablesReference", -1);
  if (variableReference > 0) {
    body["variablesReference"] = serializedResult["variablesReference"];
  }

  int namedVariables = tryGetInt(serializedResult, "namedVariables", -1);
  if (namedVariables > 0) {
    body["namedVariables"] = serializedResult["namedVariables"];
  }

  int indexedVariables = tryGetInt(serializedResult, "indexedVariables", -1);
  if (indexedVariables > 0) {
    body["indexedVariables"] = serializedResult["indexedVariables"];
  }

  try {
    const auto& presentationHint = serializedResult["presentationHint"];
    body["presentationHint"] = presentationHint;
  } catch (std::out_of_range &) {
  }

  return false;
}

std::string EvaluateCommand::prepareEvalExpression(const std::string& expr) {
  // First, trim any leading and trailing white space.
  auto const expression = trimString(expr);
  if (expression.empty()) {
    throw DebuggerCommandException("No expression provided to evaluate.");
  }

  // If the user supplied an expression that looks like a well formed script,
  // meaning it begins with <?hh, do not do any further transformations
  // on it - we'll try to just evaluate it directly as the user intended, and
  // this will honor running as PHP vs Hack. Otherwise we are going to try
  // to interpret as Hack, and we need to turn this into a valid script snippet.
  if (expression.find("<?hh", 0, 4) == 0) {
    return expression;
  }

  // HPHPD users are used to having to prefix variable requests with a leading
  // = character. We don't require that, but tolorate that syntax to maintain
  // compatibility for those users.
  if (expression[0] == '=') {
    return "<?hh " + expression.substr(1) + ";";
  }

  return "<?hh " + expression + ";";
}


}
}
