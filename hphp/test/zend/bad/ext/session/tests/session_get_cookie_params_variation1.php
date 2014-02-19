<?php

ob_start();

/* 
 * Prototype : array session_get_cookie_params(void)
 * Description : Get the session cookie parameters
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_get_cookie_params() : variation ***\n";

var_dump(session_get_cookie_params());
ini_set("session.cookie_lifetime", 3600);
var_dump(session_get_cookie_params());
ini_set("session.cookie_path", "/path");
var_dump(session_get_cookie_params());
ini_set("session.cookie_domain", "foo");
var_dump(session_get_cookie_params());
ini_set("session.cookie_secure", TRUE);
var_dump(session_get_cookie_params());
ini_set("session.cookie_httponly", TRUE);
var_dump(session_get_cookie_params());

echo "Done";
ob_end_flush();
?>