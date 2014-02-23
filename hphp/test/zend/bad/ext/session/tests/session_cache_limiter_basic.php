<?php

ob_start();

/* 
 * Prototype : string session_cache_limiter([string $cache_limiter])
 * Description : Get and/or set the current cache limiter
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_cache_limiter() : basic functionality ***\n";

var_dump(session_start());
var_dump(session_cache_limiter());
var_dump(session_cache_limiter("public"));
var_dump(session_cache_limiter());
var_dump(session_destroy());

var_dump(session_start());
var_dump(session_cache_limiter());
var_dump(session_cache_limiter("private"));
var_dump(session_cache_limiter());
var_dump(session_destroy());

var_dump(session_start());
var_dump(session_cache_limiter());
var_dump(session_cache_limiter("nocache"));
var_dump(session_cache_limiter());
var_dump(session_destroy());

var_dump(session_start());
var_dump(session_cache_limiter());
var_dump(session_cache_limiter("private_no_expire"));
var_dump(session_cache_limiter());
var_dump(session_destroy());

echo "Done";
ob_end_flush();
?>