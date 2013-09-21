<?php

ob_start();

/* 
 * Prototype : string session_encode(void)
 * Description : Encodes the current session data as a string
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_encode() : variation ***\n";

var_dump(session_start());

$array = array(1,2,3);
$_SESSION["foo"] = &$array;
$_SESSION["guff"] = &$array;
$_SESSION["blah"] = &$array;
var_dump(session_encode());
var_dump(session_destroy());

echo "Done";
ob_end_flush();
?>