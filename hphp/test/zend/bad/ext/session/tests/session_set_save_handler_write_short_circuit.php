<?php

ob_start();

/*
 * Prototype : bool session_set_save_handler(callback $open, callback $close, callback $read, callback $write, callback $destroy, callback $gc)
 * Description : Sets user-level session storage functions
 * Source code : ext/session/session.c
 */

echo "*** Testing session_set_save_handler() : test write short circuit ***\n";

require_once "save_handler.inc";
$path = dirname(__FILE__);
session_save_path($path);
session_set_save_handler("open", "close", "read", "write", "destroy", "gc");

session_start();
$session_id = session_id();
$_SESSION["Blah"] = "Hello World!";
$_SESSION["Foo"] = FALSE;
$_SESSION["Guff"] = 1234567890;
var_dump($_SESSION);

session_write_close();
session_unset();
var_dump($_SESSION);

echo "Starting session again..!\n";
session_id($session_id);
session_set_save_handler("open", "close", "read", "write", "destroy", "gc");
session_start();
var_dump($_SESSION);
$_SESSION['Bar'] = 'Foo';
session_write_close();

echo "Starting session again..!\n";
session_id($session_id);
session_set_save_handler("open", "close", "read", "write", "destroy", "gc");
session_start();
var_dump($_SESSION);
// $_SESSION should be the same and should skip write()
session_write_close();

ob_end_flush();
?>
