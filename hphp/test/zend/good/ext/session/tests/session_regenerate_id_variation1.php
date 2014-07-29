<?php

ob_start();

/* 
 * Prototype : bool session_regenerate_id([bool $delete_old_session])
 * Description : Update the current session id with a newly generated one 
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_regenerate_id() : variation ***\n";

var_dump(session_id());
var_dump(session_regenerate_id(TRUE));
var_dump(session_id());
var_dump(session_start());
var_dump(session_regenerate_id(TRUE));
var_dump(session_id());
var_dump(session_destroy());
var_dump(session_regenerate_id(TRUE));
var_dump(session_id());

echo "Done";
ob_end_flush();
?>