<?php

ob_start();

/* 
 * Prototype : string session_id([string $id])
 * Description : Get and/or set the current session id
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_id() : error functionality ***\n";

@session_start();
var_dump(session_id());
var_dump(session_id("!"));
var_dump(session_id());
@session_destroy();

@session_start();
var_dump(session_id());
var_dump(session_id("?><"));
var_dump(session_id());
@session_destroy();

@session_start();
var_dump(session_id());
var_dump(session_id("£$%^&*()"));
var_dump(session_id());
@session_destroy();

@session_start();
var_dump(session_id());
var_dump(session_id("\r\n"));
var_dump(session_id());
@session_destroy();

@session_start();
var_dump(session_id());
var_dump(session_id("\0"));
var_dump(session_id());
@session_destroy();

@session_start();
var_dump(session_id());
var_dump(session_id("¬``@~:{>?><,./[]+--"));
var_dump(session_id());
@session_destroy();

echo "Done";
ob_end_flush();
?>