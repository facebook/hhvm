<?php

ob_start();

/* 
 * Prototype : string session_encode(void)
 * Description : Encodes the current session data as a string
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_encode() : variation ***\n";

var_dump(session_encode());
var_dump(session_destroy());
var_dump(session_encode());

echo "Done";
ob_end_flush();
?>