<?php

ob_start();

/* 
 * Prototype : void session_set_cookie_params(int $lifetime [, string $path [, string $domain [, bool $secure [, bool $httponly]]]])
 * Description : Set the session cookie parameters
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_set_cookie_params() : variation ***\n";

var_dump(ini_get("session.cookie_httponly"));
var_dump(session_set_cookie_params(3600, "/path", "blah", FALSE, FALSE));
var_dump(ini_get("session.cookie_httponly"));
var_dump(session_start());
var_dump(ini_get("session.cookie_httponly"));
var_dump(session_set_cookie_params(3600, "/path", "blah", FALSE, TRUE));
var_dump(ini_get("session.cookie_httponly"));
var_dump(session_destroy());
var_dump(ini_get("session.cookie_httponly"));
var_dump(session_set_cookie_params(3600, "/path", "blah", FALSE, FALSE));
var_dump(ini_get("session.cookie_httponly"));

echo "Done";
ob_end_flush();
?>