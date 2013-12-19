<?php

ob_start();

/* 
 * Prototype : bool session_set_save_handler(callback $open, callback $close, callback $read, callback $write, callback $destroy, callback $gc)
 * Description : Sets user-level session storage functions
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_set_save_handler() : variation ***\n";

function noisy_gc($maxlifetime) {
	echo("GC [".$maxlifetime."]\n");
	gc($maxlifetime);
}

require_once "save_handler.inc";
$path = dirname(__FILE__);
session_save_path($path);
session_set_save_handler("open", "close", "read", "write", "destroy", "noisy_gc");

session_start();
$_SESSION["Blah"] = "Hello World!";
$_SESSION["Foo"] = FALSE;
$_SESSION["Guff"] = 1234567890;
var_dump($_SESSION);
$session_id = session_id();
var_dump(session_write_close());

session_set_save_handler("open", "close", "read", "write", "destroy", "noisy_gc");
session_id($session_id);
session_start();
var_dump($_SESSION);
var_dump(session_destroy());

ob_end_flush();
?>