<?php

ob_start();

/* 
 * Prototype : array session_get_cookie_params(void)
 * Description : Get the session cookie parameters
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_get_cookie_params() : basic functionality ***\n";

var_dump(session_get_cookie_params());
var_dump(session_set_cookie_params(3600, "/path", "blah", FALSE, FALSE));
var_dump(session_get_cookie_params());
var_dump(session_set_cookie_params(1234567890, "/guff", "foo", TRUE, TRUE));
var_dump(session_get_cookie_params());

echo "Done";
ob_end_flush();
?>