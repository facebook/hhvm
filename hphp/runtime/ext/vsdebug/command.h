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

#pragma once

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/vsdebug/server_object.h"
#include "hphp/runtime/vm/bytecode.h"

#include <folly/json/dynamic.h>
#include <folly/json/json.h>

namespace HPHP {
namespace VSDEBUG {

typedef int request_id_t;

// Forward declaration of Debugger
struct Debugger;
struct DebuggerSession;

// Enum describing the target to which a debugger command needs to
// be dispatched for exection.
enum CommandTarget {
  // A command targeting "Request" needs to be handed off to a particular
  // request thread that is the target of the command.
  Request,

  // A command targeting "WorkItem" needs to be handled by a request thread
  // but is internal to the debugger and does not generate a response for
  // the client.
  WorkItem,

  // A command targeting "Dummy" needs to be processed in the context of the
  // dummy request thread.
  Dummy,

  // A command targeting "None" should be executed inline as soon as
  // it is received.
  None,
};

// This serves as the base class for all VS Code Debug Protocol commands that
// can be issued from the debugger client.
struct VSCommand {
  virtual ~VSCommand();

  // Returns the name of the command.
  virtual const char* commandName() = 0;

  // Returns the target type of the command.
  virtual CommandTarget commandTarget() = 0;

  // Returns true if this command can only be executed when the program is
  // broken in to the debugger.
  virtual bool requiresBreak() = 0;

  // Returns the ID of the request thread this command is directed at.
  virtual request_id_t targetThreadId(DebuggerSession* session);

  // Executes the command. Returns true if the target thread should resume.
  bool execute();

  folly::dynamic& getMessage() { return m_message; }

  // Takes in a JSON message from the attached debugger client, parses it and
  // returns an executable debugger command.
  // Returns true if the message was successfully parsed into a command, false
  // if the message was invalid (or the command is not supported).
  static bool parseCommand(
    Debugger* debugger,
    folly::dynamic& clientMessage,
    VSCommand** command);


  // Helper routines for getting values out of a possibly malformed client
  // message. Each returns a value from the specified dynamic with the specified
  // key, or the specified default value if the key does not exist or is not
  // of the correct data type.
  static bool tryGetBool(
    const folly::dynamic& message,
    const char* key,
    bool defaultValue
  );

  static const std::string tryGetString(
    const folly::dynamic& message,
    const char* key,
    const std::string& defaultValue
  );

  static const folly::dynamic& tryGetObject(
    const folly::dynamic& message,
    const char* key,
    const folly::dynamic& defaultValue
  );

  static const folly::dynamic& tryGetArray(
    const folly::dynamic& message,
    const char* key
  );

  static int64_t tryGetInt(
    const folly::dynamic& message,
    const char* key,
    const int64_t defaultValue
  );

  static std::string trimString(const std::string str);

  static std::string removeVariableNamePrefix(const std::string& str);

  static const folly::dynamic getDebuggerCapabilities();

  static const folly::dynamic s_emptyArgs;

protected:

  // Implemented by subclasses of this object.
  virtual bool executeImpl(
    DebuggerSession* session,
    folly::dynamic* responseMsg
  ) = 0;

  // Copy of the original JSON object sent by the client.
  folly::dynamic m_message;

  Debugger* const m_debugger;

  VSCommand(Debugger* debugger, folly::dynamic message);
};

// Common implementation for all the subcommands of VSCommand so we don't have
// to repeat it in every subclass.
#define VS_COMMAND_COMMON_NOTARGET(ClassName, Target, RequiresBreak)  \
  virtual ~ClassName();                                           \
  const char* commandName() override { return #ClassName; }       \
  bool requiresBreak() override { return RequiresBreak; }         \
  ClassName(Debugger* debugger, folly::dynamic message);          \
protected:                                                        \
  bool executeImpl(                                               \
    DebuggerSession* session,                                     \
    folly::dynamic* responseMsg                                   \
  ) override;                                                     \

#define VS_COMMAND_COMMON_TARGET(ClassName, Target, RequiresBreak)  \
VS_COMMAND_COMMON_NOTARGET(ClassName, Target, RequiresBreak)      \
public:                                                           \
  CommandTarget commandTarget() override { return Target; }       \

#define VS_COMMAND_COMMON_IMPL(ClassName, Target, RequiresBreak)  \
  VS_COMMAND_COMMON_TARGET(ClassName, Target, RequiresBreak)

////// Represents an InitializeRequest command from the debugger client. //////
struct InitializeCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(InitializeCommand, CommandTarget::None, false);
};

//////  Handles both Attach/Launch commands from the debugger client. //////
struct LaunchAttachCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(LaunchAttachCommand, CommandTarget::None, false);
};

//////  Handles a continue command from the debugger client.          //////
struct ContinueCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(ContinueCommand, CommandTarget::WorkItem, false);

  // Most commands aren't to be allocated directly without a client message,
  // but resume can be generated by the backend to resume the request threads.
public:
  static ContinueCommand* createInstance(Debugger* debugger);
};

//////  Handles SetBreakpoints commands from the debugger client.     //////
struct SetBreakpointsCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(SetBreakpointsCommand, CommandTarget::None, false);
private:

  void setFnBreakpoints(
    DebuggerSession* session,
    const folly::dynamic& args,
    folly::dynamic& responseBps
  );
};

struct ResolveBreakpointsCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(
    ResolveBreakpointsCommand,
    CommandTarget::WorkItem,
    false);

public:
  static ResolveBreakpointsCommand* createInstance(Debugger* debugger);
};

//////  Handles SetExceptionBreakpoints commands                     //////
struct SetExceptionBreakpointsCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(SetExceptionBreakpointsCommand,    \
                         CommandTarget::None,               \
                         false);
};

//////  Handles StackTraceRequest from the debugger client.          //////
struct StackTraceCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(StackTraceCommand, CommandTarget::Request, false);
};

//////  Handles ConfigurationDoneRequest from the debugger client.   //////
struct ConfigurationDoneCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(ConfigurationDoneCommand, CommandTarget::None, false);
};

//////  Handles next/step requests from the debugger client.        //////
struct StepCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(StepCommand, CommandTarget::Request, true);
};

//////  Handles async break requests from the debugger client.      //////
struct PauseCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(PauseCommand, CommandTarget::None, false);
};

//////  Handles scopes requests from the debugger client.          //////
struct ScopesCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(ScopesCommand, CommandTarget::Request, false);

  request_id_t targetThreadId(DebuggerSession* session) override;

private:

  FrameObject* getFrameObject(DebuggerSession* session);

  int m_frameId;
  FrameObject* m_frameObj {nullptr};

  folly::dynamic getScopeDescription(
    DebuggerSession* session,
    const FrameObject* frame,
    const char* displayName,
    ScopeType type,
    bool expensive
  );
};

//////  Handles variables requests from the debugger client.      //////
struct VariablesCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(VariablesCommand, CommandTarget::Request, false);

  request_id_t targetThreadId(DebuggerSession* session) override;

public:

  // Returns a count of variables that are first level children of the
  // specified scope.
  static int countScopeVariables(
    DebuggerSession* session,
    Debugger* debugger,
    const ScopeObject* scope,
    request_id_t requestId
  );

  // Sorts a folly::dynamic::array of variable names in place by name.
  static void sortVariablesInPlace(folly::dynamic& vars);

  // Formats a variable's name into the PHP syntax the user expects, prepending
  // $, ::, etc.
  static const std::string getPHPVarName(const std::string& name);

  static const char* getTypeName(const Variant& variable);

  // Serializes a variable to be sent over the VS Code debugger protocol.
  static folly::dynamic serializeVariable(
    DebuggerSession* session,
    Debugger* debugger,
    request_id_t requestId,
    const std::string& name,
    const Variant& variable,
    bool doNotModifyName = false,
    folly::dynamic* presentationHint = nullptr
  );

private:

  // Helper for sorting variable names - converts a variable name to uppercase
  // for case-insensitive sort, and caches the result on the folly::dynamic
  // object to avoid doing a UC conversion on each iteration of the sort loop.
  static const std::string getUcVariableName(
    folly::dynamic& var,
    const char* ucKey
  );

  // Returns true if the variable with the specified name is a super global.
  static bool isSuperGlobal(const std::string& name);

  // Adds scope variables to the specified folly::dynamic::array, or just
  // returns a count of variables if vars == nullptr.
  static int addScopeVariables(
    DebuggerSession* session,
    Debugger* debugger,
    request_id_t requestId,
    const ScopeObject* scope,
    folly::dynamic* vars
  );

  // Adds sub scope values.
  void addSubScopes(
    VariableSubScope* subScope,
    DebuggerSession* session,
    request_id_t requestId,
    int start,
    int count,
    folly::dynamic* vars
  );

  // Adds local variables.
  static int addLocals(
    DebuggerSession* session,
    Debugger* debugger,
    request_id_t requestId,
    const ScopeObject* scope,
    folly::dynamic* vars
  );

  // Adds the specified type of constants.
  static int addConstants(
    DebuggerSession* session,
    Debugger* debugger,
    request_id_t requestId,
    const StaticString& category,
    folly::dynamic* vars
  );

  // Adds super global variables.
  static int addSuperglobalVariables(
    DebuggerSession* session,
    Debugger* debugger,
    request_id_t requestId,
    const ScopeObject* scope,
    folly::dynamic* vars
  );

  // Adds children of a complex object.
  int addComplexChildren(
    DebuggerSession* session,
    request_id_t requestId,
    int start,
    int count,
    VariableObject* variable,
    folly::dynamic* vars
  );

  // Adds array indicies.
  static int addArrayChildren(
    DebuggerSession* session,
    Debugger* debugger,
    request_id_t requestId,
    int start,
    int count,
    VariableObject* variable,
    folly::dynamic* vars
  );

  static void forEachInstanceProp(
    const Variant& var,
    std::function<bool(
      const std::string& objectClassName,
      const std::string& propName,
      const std::string& propClassName,
      const char* visibilityDescription,
      folly::dynamic& presentationHint,
      const Variant& propertyVariant
    )> callback
  );

  // Adds object properties.
  static int addObjectChildren(
    DebuggerSession* session,
    Debugger* debugger,
    request_id_t requestId,
    int start,
    int count,
    const Variant& variable,
    folly::dynamic* vars
  );

  // Adds constants defined on a class.
  static int addClassConstants(
    DebuggerSession* session,
    Debugger* debugger,
    request_id_t requestId,
    int start,
    int count,
    Class* cls,
    const Variant& var,
    folly::dynamic* vars
  );

  // Adds static properties defined on an object's class.
  static int addClassStaticProps(
    DebuggerSession* session,
    Debugger* debugger,
    request_id_t requestId,
    int start,
    int count,
    Class* cls,
    const Variant& var,
    folly::dynamic* vars
  );

  // Adds private properties defined on one of an object's base classes.
  void addClassPrivateProps(
    DebuggerSession* session,
    request_id_t requestId,
    int start,
    int count,
    VariableSubScope* subScope,
    const Variant& var,
    folly::dynamic* vars
  );

  // Adds a sub scope to an object for each class in the object's parent
  // chain (including itself) that has class constants or static props defined.
  static int addClassSubScopes(
    DebuggerSession* session,
    Debugger* debugger,
    ClassPropsType propType,
    request_id_t requestId,
    const Variant& var,
    folly::dynamic* vars
  );

  // Adds a scope sub section to a complex variable.
  static int addScopeSubSection(
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
  );

  // Tries to get a cached JSON response for a variables request
  // from the debugger session. This is used for responses that are
  // the same for all frames and requests, and are valid for the duration
  // of the current pause of the target.
  static int getCachedValue(
    DebuggerSession* session,
    const int cacheKey,
    folly::dynamic* vars
  );

  struct VariableValue {
    explicit VariableValue(const std::string &value, bool hasSummaryOverride = false)
      : m_value{value},
        m_hasSummaryOverride(hasSummaryOverride)
    {}
    const std::string m_value;
    bool m_hasSummaryOverride;
  };

  static const VariableValue getVariableValue(
    DebuggerSession* session,
    Debugger* debugger,
    request_id_t requestId,
    const Variant& variable
  );

  static const VariableValue getObjectSummary(
    DebuggerSession* session,
    Debugger* debugger,
    request_id_t requestId,
    const Object &obj
  );

  static constexpr char* VisibilityPrivate = "private";
  static constexpr char* VisibilityProtected = "protected";
  static constexpr char* VisibilityPublic = "public";

  unsigned int m_objectId;
};

//////  Handles run to location requests from the debugger client.      //////
// A run to location request is very similar to just setting a temp breakpoint
// at the specified location and removing it when it hits. However, the bp
// is never sent to the front end so it does not appear in the debugger client
// UX (no breakpoint added, removed or resolved events). Additionally, the bp
// is only added to a single request thread, and all other request threads
// remain paused.
struct RunToLocationCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(RunToLocationCommand, CommandTarget::Request, true);
};

struct SetVariableCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(SetVariableCommand, CommandTarget::Request, true);

  request_id_t targetThreadId(DebuggerSession* session) override;

private:

  bool setLocalVariable(
    DebuggerSession* session,
    const std::string& name,
    const std::string& value,
    ScopeObject* scope,
    folly::dynamic* result
  );

  bool setConstant(
    DebuggerSession* session,
    const std::string& name,
    const std::string& value,
    ScopeObject* scope,
    folly::dynamic* result
  );

  bool setArrayVariable(
    DebuggerSession* session,
    const std::string& name,
    const std::string& value,
    VariableObject* array,
    folly::dynamic* result
  );

  bool setObjectVariable(
    DebuggerSession* session,
    const std::string& name,
    const std::string& value,
    VariableObject* array,
    folly::dynamic* result
  );

  static bool getBooleanValue(const std::string& str);

  void setVariableValue(
    DebuggerSession* session,
    const std::string& name,
    const std::string& value,
    tv_lval typedVariable,
    request_id_t requestId,
    folly::dynamic* result
  );

  unsigned int m_objectId;
};

//////  Handles evaluate on call frame requests from the client      //////
struct EvaluateCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(EvaluateCommand, CommandTarget::Request, false);
  request_id_t targetThreadId(DebuggerSession* session) override;

public:
  static std::string prepareEvalExpression(const std::string& expr);

private:

  FrameObject* getFrameObject(DebuggerSession* session);
  void logToScuba(const std::string& code,
                  bool success, const std::string& error,
                  const std::string& clientId, uint32_t sessionId,
                  int64_t before, int64_t after, bool bpHit);
  int m_frameId;
  bool m_returnHhvmSerialization;
  FrameObject* m_frameObj {nullptr};
};


//////  Handles threads requests from the client                    //////
struct ThreadsCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(ThreadsCommand, CommandTarget::None, false);
};

//////  Handles terminate thread requests from the client           //////
struct TerminateThreadsCommand : public VSCommand {
  VS_COMMAND_COMMON_NOTARGET(
    TerminateThreadsCommand,
    CommandTarget::None,
    false
  );
  static TerminateThreadsCommand* createInstance(
    Debugger* debugger,
    folly::dynamic message,
    request_id_t requestId
  );

  CommandTarget commandTarget() override;
private:

  TerminateThreadsCommand(
    Debugger* debugger,
    folly::dynamic message,
    request_id_t requestId
  );

  request_id_t m_requestId;
};

//////  Handles completions requests from the client                //////
struct CompletionsCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(CompletionsCommand, CommandTarget::Request, false);
  request_id_t targetThreadId(DebuggerSession* session) override;

private:

  enum SuggestionType {
    None,
    Variable,
    Member,
    ClassStatic,
    ClassConstant,
    FuncsAndConsts
  };

  struct SuggestionContext {
    SuggestionType type;
    std::string matchPrefix;
    std::string matchContext;
  };

  struct TokenEntry {
    int tokenType;
    std::string tokenValue;
    int tokenPosition;
  };

  // Parses the specified expression using the PHP tokenizer and examines the
  // trailing tokens to determine what types of suggestions should be made.
  SuggestionContext parseCompletionExpression(const std::string& expr);

  // Adds the specified completion text and type to a list of completions
  // to be sent to the client in a CompletionsResponse message.
  static void addCompletionTarget(
    folly::dynamic& completions,
    const char* completionText,
    const char* completionType,
    int charsToOverwrite
  );

  // Adds completions based on the specified SuggestionContext for local
  // variables, and globals visible at the current VM frame for this request.
  void addVariableCompletions(
    SuggestionContext& context,
    folly::dynamic& targets
  );

  // Adds completions based on the specified SuggestionContext for properties
  // of an object obtained by evaluating the SuggestionContext's match context.
  void addMemberCompletions(
    SuggestionContext& context,
    folly::dynamic& targets
  );

  // Adds completions based on constants defined on a class whose name is the
  // SuggestionContext's match context, and any class in inherits from.
  void addClassConstantCompletions(
    SuggestionContext& context,
    folly::dynamic& targets
  );

  // Adds completions based on static members defined on a class whose name is
  // the SuggestionContext's match context, and any class in inherits from.
  void addClassStaticCompletions(
    SuggestionContext& context,
    folly::dynamic& targets
  );

  // Adds completions based on system and user defined functions visible at
  // the current fp.
  void addFuncConstantCompletions(
    SuggestionContext& context,
    folly::dynamic& targets
  );

  // Adds the specified name as a completion option to targets if the name
  // has 1+ prefix characters in common with matchPrefix.
  void addIfMatch(
    const std::string& name,
    const std::string& matchPrefix,
    const char* type,
    folly::dynamic& targets
  );

  // Completion suggestion types, from protocol.
  static constexpr char* CompletionTypeFn = "function";
  static constexpr char* CompletionTypeVar = "variable";
  static constexpr char* CompletionTypeClass = "class";
  static constexpr char* CompletionTypeProp = "property";
  static constexpr char* CompletionTypeKeyword = "keyword";
  static constexpr char* CompletionTypeValue = "value";

  FrameObject* getFrameObject(DebuggerSession* session);
  int m_frameId;
  FrameObject* m_frameObj {nullptr};
};

////// Represents an InitializeRequest command from the debugger client. //////
struct InfoCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(InfoCommand, CommandTarget::Request, false);
};

#undef VS_COMMAND_COMMON_IMPL

}
}
