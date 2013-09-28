<?php

ob_start();

/* 
 * Prototype : string session_id([string $id])
 * Description : Get and/or set the current session id
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_id() : basic functionality ***\n";

var_dump(session_id());
var_dump(session_id("test"));
var_dump(session_id());
var_dump(session_id("1234567890"));
var_dump(session_id());
// Turn off strice mode, since it does not allow uninitialized session ID
ini_set('session.use_strict_mode',false);
var_dump(session_start());
var_dump(session_id());
var_dump(session_destroy());
var_dump(session_id());

echo "Done";
ob_end_flush();
?>