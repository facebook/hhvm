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

#ifndef incl_HPHP_VSDEBUG_COMMAND_H_
#define incl_HPHP_VSDEBUG_COMMAND_H_

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/vsdebug/server_object.h"

#include <folly/dynamic.h>
#include <folly/json.h>

namespace HPHP {
namespace VSDEBUG {

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
  virtual int64_t targetThreadId(DebuggerSession* session);

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

  static const std::string& tryGetString(
    const folly::dynamic& message,
    const char* key,
    const std::string& defaultValue
  );

  static const folly::dynamic& tryGetObject(
    const folly::dynamic& message,
    const char* key,
    const folly::dynamic& defaultValue
  );

  static int64_t tryGetInt(
    const folly::dynamic& message,
    const char* key,
    const int64_t defaultValue
  );

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

  static const folly::dynamic s_emptyArgs;
};

// Common implementation for all the subcommands of VSCommand so we don't have
// to repeat it in every subclass.
#define VS_COMMAND_COMMON_IMPL(ClassName, Target, RequiresBreak)  \
  virtual ~ClassName();                                           \
  const char* commandName() override { return #ClassName; }       \
  CommandTarget commandTarget() override { return Target; }       \
  bool requiresBreak() override { return RequiresBreak; }         \
  ClassName(Debugger* debugger, folly::dynamic message);          \
protected:                                                        \
  bool executeImpl(                                               \
    DebuggerSession* session,                                     \
    folly::dynamic* responseMsg                                   \
  ) override;                                                     \


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

  int64_t targetThreadId(DebuggerSession* session) override;

private:

  FrameObject* getFrameObject(DebuggerSession* session);

  unsigned int m_frameId;
  FrameObject* m_frameObj {nullptr};

  folly::dynamic getScopeDescription(
    DebuggerSession* session,
    const char* displayName,
    ScopeType type
  );
};

//////  Handles variables requests from the debugger client.      //////
struct VariablesCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(VariablesCommand, CommandTarget::Request, false);

  int64_t targetThreadId(DebuggerSession* session) override;

public:

  // Returns a count of variables that are first level children of the
  // specified scope.
  static int countScopeVariables(const ScopeObject* scope);

  // Sorts a folly::dynamic::array of variable names in place by name.
  static void sortVariablesInPlace(folly::dynamic& vars);

  // Formats a variable's name into the PHP syntax the user expects, prepending
  // $, ::, etc.
  static const std::string getPHPVarName(const std::string& name);

  static const char* getTypeName(const Variant& variable);

private:

  // Helper for sorting variable names - converts a variable name to uppercase
  // for case-insensitive sort, and caches the result on the folly::dynamic
  // object to avoid doing a UC conversion on each iteration of the sort loop.
  static const std::string& getUcVariableName(
    folly::dynamic& var,
    const char* ucKey
  );

  // Returns true if the variable with the specified name is a super global.
  static bool isSuperGlobal(const std::string& name);

  // Adds scope variables to the specified folly::dynamic::array, or just
  // returns a count of variables if vars == nullptr.
  static int addScopeVariables(
    DebuggerSession* session,
    int64_t requestId,
    const ScopeObject* scope,
    folly::dynamic* vars
  );

  // Adds local variables.
  static int addLocals(
    DebuggerSession* session,
    int64_t requestId,
    const ScopeObject* scope,
    folly::dynamic* vars
  );

  // Adds the specified type of constants.
  static int addConstants(
    DebuggerSession* session,
    int64_t requestId,
    const ScopeObject* scope,
    const StaticString& category,
    folly::dynamic* vars
  );

  // Adds super global variables.
  static int addSuperglobalVariables(
    DebuggerSession* session,
    int64_t requestId,
    const ScopeObject* scope,
    folly::dynamic* vars
  );

  // Adds children of a complex object.
  static int addComplexChildren(
    DebuggerSession* session,
    int64_t requestId,
    int start,
    int count,
    VariableObject* variable,
    folly::dynamic* vars
  );

  // Adds array indicies.
  static int addArrayChildren(
    DebuggerSession* session,
    int64_t requestId,
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
      const std::string& displayName,
      const char* visibilityDescription,
      folly::dynamic& presentationHint,
      const Variant& propertyVariant
    )> callback
  );

  // Adds object properties.
  static int addObjectChildren(
    DebuggerSession* session,
    int64_t requestId,
    int start,
    int count,
    const Variant& variable,
    folly::dynamic* vars
  );

  // Adds constants defined on a class.
  static int addClassConstants(
    DebuggerSession* session,
    int64_t requestId,
    int start,
    int count,
    Class* cls,
    const Variant& var,
    folly::dynamic* vars
  );

  // Adds static properties defined on an object's class.
  static int addClassStaticProps(
    DebuggerSession* session,
    int64_t requestId,
    int start,
    int count,
    Class* cls,
    const Variant& var,
    folly::dynamic* vars
  );

  // Serializes a variable to be sent over the VS Code debugger protocol.
  static folly::dynamic serializeVariable(
    DebuggerSession* session,
    int64_t requestId,
    const std::string& name,
    const Variant& variable,
    bool doNotModifyName = false,
    folly::dynamic* presentationHint = nullptr
  );

  // Adds private properties defined on one of an object's base classes.
  static void addClassPrivateProps(
    DebuggerSession* session,
    int64_t requestId,
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
    ClassPropsType propType,
    int requestId,
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
    int requestId,
    const Variant& var,
    folly::dynamic* vars
  );

  static const std::string getVariableValue(const Variant& variable);

  static constexpr char* VisibilityPrivate = "private";
  static constexpr char* VisibilityProtected = "protected";
  static constexpr char* VisibilityPublic = "public";

  unsigned int m_objectId;
};

#undef VS_COMMAND_COMMON_IMPL

}
}

#endif // incl_HPHP_VSDEBUG_COMMAND_H_
