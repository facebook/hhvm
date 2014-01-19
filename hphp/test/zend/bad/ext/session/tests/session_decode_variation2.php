<?php

ob_start();

/* 
 * Prototype : string session_decode(void)
 * Description : Decodes session data from a string
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_decode() : variation ***\n";

var_dump(session_start());
var_dump($_SESSION);
$_SESSION["foo"] = 1234567890;
$_SESSION["bar"] = "Hello World!";
$_SESSION["guff"] = 123.456;
var_dump($_SESSION);
var_dump(session_decode("foo|a:3:{i:0;i:1;i:1;i:2;i:2;i:3;}guff|R:1;blah|R:1;"));
var_dump($_SESSION);
var_dump(session_destroy());

echo "Done";
ob_end_flush();
?>