<?php

ob_start();

/* 
 * Prototype : bool session_set_save_handler(callback $open, callback $close, callback $read, callback $write, callback $destroy, callback $gc)
 * Description : Sets user-level session storage functions
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_set_save_handler() : error functionality ***\n";

function callback() { }

session_set_save_handler("callback", "callback", "callback", "callback", "callback", "callback");
session_set_save_handler("callback", "echo", "callback", "callback", "callback", "callback");
session_set_save_handler("callback", "callback", "echo", "callback", "callback", "callback");
session_set_save_handler("callback", "callback", "callback", "echo", "callback", "callback");
session_set_save_handler("callback", "callback", "callback", "callback", "echo", "callback");
session_set_save_handler("callback", "callback", "callback", "callback", "callback", "echo");
session_set_save_handler("callback", "callback", "callback", "callback", "callback", "callback");
session_start();
ob_end_flush();
?>