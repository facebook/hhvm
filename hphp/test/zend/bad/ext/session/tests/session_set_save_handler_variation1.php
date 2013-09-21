<?php

ob_start();

/* 
 * Prototype : bool session_set_save_handler(callback $open, callback $close, callback $read, callback $write, callback $destroy, callback $gc)
 * Description : Sets user-level session storage functions
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_set_save_handler() : variation ***\n";

var_dump(session_module_name());
var_dump(session_module_name(FALSE));
var_dump(session_module_name());
var_dump(session_module_name("blah"));
var_dump(session_module_name());
var_dump(session_module_name("files"));
var_dump(session_module_name());

ob_end_flush();
?>