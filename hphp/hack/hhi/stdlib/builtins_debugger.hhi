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
  HH\FIXME\MISSING_PARAM_TYPE $cmd,
  HH\FIXME\MISSING_PARAM_TYPE $clsname,
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
function hphp_was_interrupted_by_debugger()[read_globals]: bool;
<<__PHPStdLib>>
function hphpd_get_client(
  HH\FIXME\MISSING_PARAM_TYPE $name = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hphpd_client_ctrl(
  HH\FIXME\MISSING_PARAM_TYPE $name,
  HH\FIXME\MISSING_PARAM_TYPE $op,
): HH\FIXME\MISSING_RETURN_TYPE;
