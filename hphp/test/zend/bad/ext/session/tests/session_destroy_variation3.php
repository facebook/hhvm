<?php

ob_start();

/* 
 * Prototype : bool session_destroy(void)
 * Description : Destroys all data registered to a session
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_destroy() : variation ***\n";

var_dump(session_start());
var_dump(session_id());
var_dump(session_destroy());
var_dump(session_id());
var_dump(session_start());
var_dump(session_id());
var_dump(session_destroy());
var_dump(session_id());

echo "Done";
ob_end_flush();
?>