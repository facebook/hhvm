<?php

ob_start();

/* 
 * Prototype : string session_module_name([string $module])
 * Description : Get and/or set the current session module
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_module_name() : variation ***\n";

require_once "save_handler.inc";
$path = dirname(__FILE__);
session_save_path($path);
session_module_name("files");

session_start();
$_SESSION["Blah"] = "Hello World!";
$_SESSION["Foo"] = FALSE;
$_SESSION["Guff"] = 1234567890;
var_dump($_SESSION);

var_dump(session_write_close());
session_start();
var_dump($_SESSION);
var_dump(session_destroy());
session_start();
var_dump($_SESSION);
var_dump(session_destroy());

ob_end_flush();
?>