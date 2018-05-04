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

#ifndef incl_HPHP_CLI_SERVER_H_
#define incl_HPHP_CLI_SERVER_H_

/*
 * CLI Server
 *
 * This module hosts a server on a local socket which can be used to execute
 * PHP scripts. The server will delegate file system operations as well as
 * proc_open commands to the client, allowing the server to masquerade as the
 * client user.
 *
 * The main advantage here is that clients can share a single translation cache
 * and APC cache, and avoid paying the cost of hhvm startup with each script
 * invocation.
 *
 * Using the CLI server requires no changes to the permission model of the
 * server, and as privileged access is delegated to clients the risk of
 * privilege escalation within the server is minimized. Additionally the server
 * itself is only accessible via local socket, but can share a translation
 * cache with requests executing via the webserver.
 *
 * This server is still experimental and in particular there is very limited
 * transfer of configuration settings from the client. As a result scripts
 * executed via this module will be running with settings largely defined by
 * the server process. Currently PHP_INI_USER, PHP_INI_ALL, ServerVariables,
 * EnvVariables, and the remote environment are transferred.
 *
 * Access to the CLI server can be controlled by the runtime options
 * UnixServerAllowed{Users,Groups}. When both arrays are empty all users will
 * be allowed, otherwise only enumerated users and users with membership in
 * enumerated groups will be permitted access.
 *
 * Unit loading within the CLI server is controlled via two different runtime
 * options. Unlike ordinary file access the server will attempt to load all
 * units directly before falling back to the client.
 *
 * When UnixServerQuarantineUnits is set units not opened directly by the server
 * process will be written to a per-user unit cache only used for CLI server
 * requests by that user.
 *
 * When UnixServerVerifyExeAccess is set the server will verify that the client
 * can read each unit before loading them. The client is required to send the
 * server a read file descriptor for the unit, which the server will verify is
 * open in read mode using fcntl, has inode and device numbers matching those
 * seen by the server when executing stat.
 *
 * A UnixServerQuarantineApc is also available, which forces all apc operations
 * performed by the CLI server to use a per user cache not shared with the
 * webserver.
 */

#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/util/trace.h"

#include <folly/portability/Sockets.h>

#include <string>
#include <vector>

namespace HPHP {

#ifndef SCM_CREDENTIALS
struct ucred {
  pid_t pid;
  uid_t uid;
  gid_t gid;
};
#endif

/*
 * The API version of the CLI-Server protocol
 */
extern const uint64_t CLI_SERVER_API_VERSION;

/*
 * Returns true if the current request is executing in CLI mode
 */
bool is_cli_mode();

/*
 * Create a unix socket at socket_path and begin running a thread to handle
 * connection events. The socket will be writable by the world (connecting to
 * a local socket requires write access).
 */
void init_cli_server(const char* socket_path);

/*
 * Begin accepting new connections to the CLI server created via a call to
 * init_cli_server().
 */
void start_cli_server();

/*
 * Shutdown the CLI server and wait for outstanding requests to terminate.
 */
void teardown_cli_server();

/*
 * Run "file" on the CLI server accessible via sock_path with argument vector
 * argv.
 *
 * The command will return iff the CLI server is unreachable.
 */
void run_command_on_cli_server(const char* sock_path,
                               const std::vector<std::string>& args,
                               int& count);

/*
 * Returns the thread local ucred structure if the active thread is executing a
 * CLI request.
 */
ucred* get_cli_ucred();

/*
 * Perform mkstemp(buf) on the connected CLI client.
 */
bool cli_mkstemp(char* buf);

/*
 * Explicitly open a file via the CLI-server client.
 *
 * WARNING: use of this function should be considered a last resort, it's
 * much better to get a CLIWrapper instance and use that. The problem with
 * getting a raw fd is reads/writes will be unchecked, and if the client dies
 * (e.g. the user presses ^C to "kill the script") nothing will immediately stop
 * IO happening on this fd. So, the user may experience surprising behavior
 * where files change even after the script is "dead".
 */
int cli_openfd_unsafe(const String& filename, int flags, mode_t mode,
                      bool use_include_path, bool quiet);

void run_command_on_cli_client(const std::string& name,
                               std::function<void(int)> cmd);

/*
 * Fetch the environment for the CLI process.
 */
Array cli_env();

bool is_cli_server_mode();

bool should_use_cli_hhvm_fe();

#define CLI_HHVM_FE(NAME)                                                      \
  {                                                                            \
    if (should_use_cli_hhvm_fe())                                              \
    {                                                                          \
      OverCLI::RegisterClientSideFunc BOOST_PP_CAT(reg_, NAME)(                \
        BOOST_PP_STRINGIZE(NAME), BOOST_PP_CAT(client_, NAME));                \
    }                                                                          \
  }                                                                            \
  HHVM_FE(NAME)

// All args are must be either const or non-reference.
// (we dont want them mutable). This is relevant to the three macros below.
#define CLI_HHVM_FUNCTION(TYPE, NAME, ...)                                     \
  TYPE BOOST_PP_CAT(_cli_impl_, NAME)(                                         \
    GENERATE_PARAMETERS(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)));               \
  void BOOST_PP_CAT(client_, NAME)(int fd) {                                   \
    GENERATE_CLIENT_VARIABLES(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__));          \
    send_over_wire(                                                            \
      fd, BOOST_PP_CAT(_cli_impl_, NAME)(                                      \
        GENERATE_ARGUMENTS(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))));           \
  }                                                                            \
  TYPE HHVM_FUNCTION(                                                          \
    NAME, GENERATE_PARAMETERS(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))) {        \
    if (is_cli_server_mode()) {                                                \
      TYPE ret;                                                                \
      run_command_on_cli_client(BOOST_PP_STRINGIZE(NAME), [&](int fd) {        \
        GENERATE_SERVER_SEND(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))            \
        receive_over_wire(fd, ret);                                            \
      });                                                                      \
      return ret;                                                              \
    }                                                                          \
    return BOOST_PP_CAT(_cli_impl_, NAME)(                                     \
      GENERATE_ARGUMENTS(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)));              \
  }                                                                            \
  TYPE BOOST_PP_CAT(_cli_impl_, NAME)(                                         \
    GENERATE_PARAMETERS(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

// The following macro defines all the necessary functionality
// for executing no params function on the client.
#define CLI_HHVM_NO_PARAM_FUNCTION(TYPE, NAME)                                 \
  TYPE BOOST_PP_CAT(_cli_impl_, NAME)();                                       \
  void BOOST_PP_CAT(client_, NAME)(int fd) {                                   \
    send_over_wire(                                                            \
      fd, BOOST_PP_CAT(_cli_impl_, NAME)());                                   \
  }                                                                            \
  TYPE HHVM_FUNCTION(NAME) {                                                   \
    if (is_cli_server_mode()) {                                                \
      TYPE ret;                                                                \
      run_command_on_cli_client(BOOST_PP_STRINGIZE(NAME), [&](int fd) {        \
        receive_over_wire(fd, ret);                                            \
      });                                                                      \
      return ret;                                                              \
    }                                                                          \
    return BOOST_PP_CAT(_cli_impl_, NAME)();                                   \
  }                                                                            \
  TYPE BOOST_PP_CAT(_cli_impl_, NAME)()

// The following macro defines all the necessary functionality
// for executing void functions on the client.
#define CLI_HHVM_VOID_FUNCTION(NAME, ...)                                      \
  void BOOST_PP_CAT(_cli_impl_, NAME)(                                         \
    GENERATE_PARAMETERS(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)));               \
  void BOOST_PP_CAT(client_, NAME)(int fd) {                                   \
    GENERATE_CLIENT_VARIABLES(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__));          \
    BOOST_PP_CAT(_cli_impl_, NAME)(                                            \
      GENERATE_ARGUMENTS(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)));              \
  }                                                                            \
  void HHVM_FUNCTION(                                                          \
    NAME, GENERATE_PARAMETERS(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))) {        \
    if (is_cli_server_mode()) {                                                \
      run_command_on_cli_client(BOOST_PP_STRINGIZE(NAME), [&](int fd) {        \
        GENERATE_SERVER_SEND(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))            \
      });                                                                      \
      return;                                                                  \
    }                                                                          \
    BOOST_PP_CAT(_cli_impl_, NAME)(                                            \
      GENERATE_ARGUMENTS(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)));              \
  }                                                                            \
  void BOOST_PP_CAT(_cli_impl_, NAME)(                                         \
    GENERATE_PARAMETERS(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

#define GENERATE_PARAMETERS(Args)                                              \
  BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(MAKE_PARAMETER, % %, Args))

#define GENERATE_ARGUMENTS(Args)                                               \
  BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(MAKE_ARGUMENT, % %, Args))

#define GENERATE_CLIENT_VARIABLES(Args)                                        \
  BOOST_PP_SEQ_FOR_EACH(MAKE_CLIENT_VARIABLE, ;, Args)

#define GENERATE_SERVER_SEND(Args)                                             \
  BOOST_PP_SEQ_FOR_EACH(MAKE_SERVER_SEND_VARIABLE, ;, Args)

#define MAKE_PARAMETER(s, Unused, Arg)                                         \
  BOOST_PP_TUPLE_ELEM(2, 0, Arg) BOOST_PP_TUPLE_ELEM(2, 1, Arg)

#define MAKE_ARGUMENT(s, Unused, Arg) BOOST_PP_TUPLE_ELEM(2, 1, Arg)

#define MAKE_CLIENT_VARIABLE(s, Sep, Arg)                                      \
  std::remove_cv<std::remove_reference<BOOST_PP_TUPLE_ELEM(                    \
    2, 0, Arg)>::type>::type BOOST_PP_TUPLE_ELEM(2, 1, Arg);                   \
  receive_over_wire(fd, BOOST_PP_TUPLE_ELEM(2, 1, Arg)) Sep

#define MAKE_SERVER_SEND_VARIABLE(s, Sep, Arg)                                 \
  send_over_wire(fd, std::move(BOOST_PP_TUPLE_ELEM(2, 1, Arg))) Sep

#define CLI_HHVM_FALIAS(fn, falias)                                            \
  {                                                                            \
    if (should_use_cli_hhvm_fe())                                              \
    {                                                                          \
      OverCLI::RegisterClientSideFunc BOOST_PP_CAT(reg_, falias)(              \
        BOOST_PP_STRINGIZE(falias),                                            \
        BOOST_PP_CAT(client_, falias));                                        \
    }                                                                          \
  }                                                                            \
  HHVM_FALIAS(fn, falias)

namespace OverCLI {

void RunClientCommand(const std::string& cmd, int fd);
struct RegisterClientSideFunc {
  RegisterClientSideFunc(const std::string& a, std::function<void(int)> f);
  static void finishedAllRegistering();
};

struct ClientSideResource : ResourceData {
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(ClientSideResource);
  int index;
  explicit ClientSideResource(int id) : index(id) {}

  private:
  static req::hash_map<int, Resource>& getMapping() {
    static req::hash_map<int, Resource> mappings;
    return mappings;
  }

  public:
  static Resource& fromIndex(int id) {
    assert(getMapping().count(id) != 0);
    return getMapping()[id];
  }

  static int fromResource(Resource&& v) {
    static int id = 0;
    assert(getMapping().count(id) == 0); // otherwise the mapping is corrupted
    // e.g. either an invalid ClientSideResource was sent here or
    // a resource from another place was sent here
    int my_id = id++;
    getMapping()[my_id] = std::move(v);
    return my_id;
  }
};
} // namespace OverCLI

void receive_over_wire(int fd, VRefParam& vrp);
void receive_over_wire(int fd, Array& a);
void receive_over_wire(int fd, Variant& v);
void receive_over_wire(int fd, Resource& r);
void receive_over_wire(int fd, String& s);
void receive_over_wire(int fd, int& v);
void receive_over_wire(int fd, bool& v);
void receive_over_wire(int fd, int64_t& v);
void receive_over_wire(int fd, double& d);

void send_over_wire(int fd, VRefParam& vrp);
void send_over_wire(int fd, const Array& a);
void send_over_wire(int fd, const Variant& v);
void send_over_wire(int fd, const Resource& r);
void send_over_wire(int fd, const String& s);
void send_over_wire(int fd, const int v);
void send_over_wire(int fd, const bool v);
void send_over_wire(int fd, const int64_t v);
void send_over_wire(int fd, const double d);

} // namespace HPHP

#endif
