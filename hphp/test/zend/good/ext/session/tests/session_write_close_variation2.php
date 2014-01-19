<?php

ob_start();

/* 
 * Prototype : bool session_write_close(void)
 * Description : Write session data and end session
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_write_close() : variation ***\n";

var_dump(session_start());
var_dump($_SESSION);
var_dump(session_write_close());
var_dump($_SESSION);
var_dump(session_start());
var_dump($_SESSION);
var_dump(session_write_close());
var_dump($_SESSION);
var_dump(session_start());
var_dump($_SESSION);
var_dump(session_write_close());
var_dump($_SESSION);
var_dump(session_start());
var_dump(session_destroy());

echo "Done";
ob_end_flush();
?>