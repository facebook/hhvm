<?php

ob_start();

/* 
 * Prototype : bool session_start(void)
 * Description : Initialize session data
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_start() : variation ***\n";

var_dump(session_start());
var_dump(session_write_close());
var_dump(session_start());
var_dump(session_write_close());
var_dump(session_start());
var_dump(session_write_close());
var_dump(session_start());
var_dump(session_write_close());
var_dump(session_start());
var_dump(session_write_close());
var_dump(session_destroy());

echo "Done";
ob_end_flush();
?>