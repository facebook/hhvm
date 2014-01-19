<?php

ob_start();

/* 
 * Prototype : bool session_set_save_handler(callback $open, callback $close, callback $read, callback $write, callback $destroy, callback $gc)
 * Description : Sets user-level session storage functions
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_set_save_handler() : using closures as callbacks ***\n";

require_once "save_handler_closures.inc";
var_dump(session_module_name());
var_dump(session_module_name(FALSE));
var_dump(session_module_name("blah"));
var_dump(session_module_name("foo"));

$path = dirname(__FILE__);
session_save_path($path);
session_set_save_handler($open_closure, $close_closure, $read_closure, $write_closure, $destroy_closure, $gc_closure);

session_start();
$_SESSION["Blah"] = "Hello World!";
$_SESSION["Foo"] = FALSE;
$_SESSION["Guff"] = 1234567890;
var_dump($_SESSION);

session_write_close();
session_unset();
var_dump($_SESSION);

echo "Starting session again..!\n";
session_id($session_id);
session_set_save_handler($open_closure, $close_closure, $read_closure, $write_closure, $destroy_closure, $gc_closure);
session_start();
var_dump($_SESSION);
session_write_close();

ob_end_flush();
?>