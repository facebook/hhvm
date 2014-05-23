<?php

ob_start();

/* 
 * Prototype : string session_encode(void)
 * Description : Encodes the current session data as a string
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_encode() : variation ***\n";

var_dump(session_start());
$_SESSION["foo"] = 1234567890;
$encoded = session_encode();
var_dump(base64_encode($encoded));
var_dump(session_destroy());

echo "Done";
ob_end_flush();
?>