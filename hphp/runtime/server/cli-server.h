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

#include "hphp/runtime/base/type-array.h"

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
                               const std::vector<std::string>& args);

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
 * Fetch the environment for the CLI process.
 */
Array cli_env();

}

#endif
