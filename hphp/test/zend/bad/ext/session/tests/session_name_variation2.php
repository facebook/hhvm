<?php

ob_start();

/* 
 * Prototype : string session_name([string $name])
 * Description : Get and/or set the current session name
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_name() : variation ***\n";

var_dump(session_name());
var_dump(session_name("blah"));
var_dump(session_start());
var_dump(session_name());
var_dump(session_destroy());
var_dump(session_name());

echo "Done";
ob_end_flush();
?>