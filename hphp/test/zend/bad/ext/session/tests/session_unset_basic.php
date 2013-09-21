<?php

ob_start();

/* 
 * Prototype : void session_unset(void)
 * Description : Free all session variables
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_unset() : basic functionality ***\n";

var_dump(session_start());
$_SESSION["foo"] = "Hello World!";
var_dump($_SESSION);
var_dump(session_unset());
var_dump($_SESSION);
var_dump(session_destroy());
var_dump($_SESSION);

echo "Done";
ob_end_flush();
?>