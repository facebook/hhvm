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
function hphpd_install_user_command(
  $cmd,
  $clsname,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphpd_auth_token(): string;
<<__PHPStdLib>>
function hphp_debug_session_auth(): string;
<<__PHPStdLib>>
function hphpd_get_user_commands(): HH\FIXME\MISSING_RETURN_TYPE;
function hphpd_break(bool $condition = true)[]: void;
function hphp_debug_break(bool $condition = true)[]: bool;
<<__PHPStdLib>>
function hphp_debugger_attached()[read_globals]: bool;
<<__PHPStdLib>>
function hphpd_get_client($name = null): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphpd_client_ctrl($name, $op): HH\FIXME\MISSING_RETURN_TYPE;
class DebuggerProxyCmdUser {
  public function __construct();
  public function isLocal(): HH\FIXME\MISSING_RETURN_TYPE;
  public function send($cmd): HH\FIXME\MISSING_RETURN_TYPE;
}
class DebuggerClientCmdUser {
  const int AUTO_COMPLETE_FILENAMES = 0;
  const int AUTO_COMPLETE_VARIABLES = 0;
  const int AUTO_COMPLETE_CONSTANTS = 0;
  const int AUTO_COMPLETE_CLASSES = 0;
  const int AUTO_COMPLETE_FUNCTIONS = 0;
  const int AUTO_COMPLETE_CLASS_METHODS = 0;
  const int AUTO_COMPLETE_CLASS_PROPERTIES = 0;
  const int AUTO_COMPLETE_CLASS_CONSTANTS = 0;
  const int AUTO_COMPLETE_KEYWORDS = 0;
  const int AUTO_COMPLETE_CODE = 0;
  public function __construct();
  public function quit(): HH\FIXME\MISSING_RETURN_TYPE;
  public function help($format, ...$args): HH\FIXME\MISSING_RETURN_TYPE;
  public function info($format, ...$args): HH\FIXME\MISSING_RETURN_TYPE;
  public function output($format, ...$args): HH\FIXME\MISSING_RETURN_TYPE;
  public function error($format, ...$args): HH\FIXME\MISSING_RETURN_TYPE;
  public function code(
    $source,
    $highlight_line = 0,
    $start_line_no = 0,
    $end_line_no = 0,
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function ask($format, ...$args): HH\FIXME\MISSING_RETURN_TYPE;
  public function wrap($str): HH\FIXME\MISSING_RETURN_TYPE;
  public function helpTitle($str): HH\FIXME\MISSING_RETURN_TYPE;
  public function helpCmds(
    $cmd,
    $desc,
    ...$args
  ): HH\FIXME\MISSING_RETURN_TYPE;
  public function helpBody($str): HH\FIXME\MISSING_RETURN_TYPE;
  public function helpSection($str): HH\FIXME\MISSING_RETURN_TYPE;
  public function tutorial($str): HH\FIXME\MISSING_RETURN_TYPE;
  public function getCode(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getCommand(): HH\FIXME\MISSING_RETURN_TYPE;
  public function arg($index, $str): HH\FIXME\MISSING_RETURN_TYPE;
  public function argCount(): HH\FIXME\MISSING_RETURN_TYPE;
  public function argValue($index): HH\FIXME\MISSING_RETURN_TYPE;
  public function lineRest($index): HH\FIXME\MISSING_RETURN_TYPE;
  public function args(): HH\FIXME\MISSING_RETURN_TYPE;
  public function send($cmd): HH\FIXME\MISSING_RETURN_TYPE;
  public function xend($cmd): HH\FIXME\MISSING_RETURN_TYPE;
  public function getCurrentLocation(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getStackTrace(): HH\FIXME\MISSING_RETURN_TYPE;
  public function getFrame(): HH\FIXME\MISSING_RETURN_TYPE;
  public function printFrame($index): HH\FIXME\MISSING_RETURN_TYPE;
  public function addCompletion($list): HH\FIXME\MISSING_RETURN_TYPE;
}
class DebuggerClient {
  const int STATE_INVALID = 0;
  const int STATE_UNINIT = 0;
  const int STATE_INITIALIZING = 0;
  const int STATE_READY_FOR_COMMAND = 0;
  const int STATE_BUSY = 0;
  public function __construct();
  public function getState(): HH\FIXME\MISSING_RETURN_TYPE;
  public function init($options): HH\FIXME\MISSING_RETURN_TYPE;
  public function processCmd($cmdName, $args): HH\FIXME\MISSING_RETURN_TYPE;
}
