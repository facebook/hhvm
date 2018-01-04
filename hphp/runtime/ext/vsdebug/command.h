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
  virtual int64_t targetThreadId() = 0;

  // Executes the command. Returns true if the target thread should resume.
  bool execute();

  folly::dynamic& getMessage() { return m_message; }

  // A default implementation of getting the request's target thread ID that
  // applies to most types of request methods.
  int64_t defaultGetTargetThreadId();

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
  int64_t targetThreadId() override;                              \
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
//////  Handles ConfigurationDoneRequest from the debugger client.   //////
struct ConfigurationDoneCommand : public VSCommand {
  VS_COMMAND_COMMON_IMPL(ConfigurationDoneCommand, CommandTarget::None, false);
};

#undef VS_COMMAND_COMMON_IMPL

}
}

#endif // incl_HPHP_VSDEBUG_COMMAND_H_
