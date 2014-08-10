<?php

ob_start();

/* 
 * Prototype : string session_encode(void)
 * Description : Encodes the current session data as a string
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_encode() : variation ***\n";

var_dump(session_start());
$_SESSION[] = 1234567890;
var_dump(session_encode());
var_dump(session_destroy());
var_dump(session_start());
$_SESSION[1234567890] = "Hello World!";
var_dump(session_encode());
var_dump(session_destroy());
var_dump(session_start());
$_SESSION[-1234567890] = 1234567890;
var_dump(session_encode());
var_dump(session_destroy());

echo "Done";
ob_end_flush();
?>
