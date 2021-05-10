<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function hphpd_install_user_command($cmd, $clsname);
<<__PHPStdLib>>
function hphpd_auth_token(): string;
<<__PHPStdLib>>
function hphp_debug_session_auth(): string;
<<__PHPStdLib>>
function hphpd_get_user_commands();
function hphpd_break(bool $condition = true): void;
function hphp_debug_break(bool $condition = true): bool;
<<__PHPStdLib>>
function hphp_debugger_attached()[read_globals]: bool;
<<__PHPStdLib>>
function hphpd_get_client($name = null);
<<__PHPStdLib>>
function hphpd_client_ctrl($name, $op);
class DebuggerProxyCmdUser {
  public function __construct();
  public function isLocal();
  public function send($cmd);
}
class DebuggerClientCmdUser {
  const AUTO_COMPLETE_FILENAMES = 0;
  const AUTO_COMPLETE_VARIABLES = 0;
  const AUTO_COMPLETE_CONSTANTS = 0;
  const AUTO_COMPLETE_CLASSES = 0;
  const AUTO_COMPLETE_FUNCTIONS = 0;
  const AUTO_COMPLETE_CLASS_METHODS = 0;
  const AUTO_COMPLETE_CLASS_PROPERTIES = 0;
  const AUTO_COMPLETE_CLASS_CONSTANTS = 0;
  const AUTO_COMPLETE_KEYWORDS = 0;
  const AUTO_COMPLETE_CODE = 0;
  public function __construct();
  public function quit();
  public function help($format, ...$args);
  public function info($format, ...$args);
  public function output($format, ...$args);
  public function error($format, ...$args);
  public function code($source, $highlight_line = 0, $start_line_no = 0, $end_line_no = 0);
  public function ask($format, ...$args);
  public function wrap($str);
  public function helpTitle($str);
  public function helpCmds($cmd, $desc, ...$args);
  public function helpBody($str);
  public function helpSection($str);
  public function tutorial($str);
  public function getCode();
  public function getCommand();
  public function arg($index, $str);
  public function argCount();
  public function argValue($index);
  public function lineRest($index);
  public function args();
  public function send($cmd);
  public function xend($cmd);
  public function getCurrentLocation();
  public function getStackTrace();
  public function getFrame();
  public function printFrame($index);
  public function addCompletion($list);
}
class DebuggerClient {
  const STATE_INVALID = 0;
  const STATE_UNINIT = 0;
  const STATE_INITIALIZING = 0;
  const STATE_READY_FOR_COMMAND = 0;
  const STATE_BUSY = 0;
  public function __construct();
  public function getState();
  public function init($options);
  public function processCmd($cmdName, $args);
}
