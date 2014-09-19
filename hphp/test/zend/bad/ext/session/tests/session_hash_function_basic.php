<?php

ob_start();

echo "*** Testing session.hash_function : basic functionality ***\n";

var_dump(ini_set('session.hash_function', 'md5'));
var_dump(session_start());
var_dump(!empty(session_id()), session_id());
var_dump(session_destroy());

var_dump(ini_set('session.hash_function', 'sha1'));
var_dump(session_start());
var_dump(!empty(session_id()), session_id());
var_dump(session_destroy());

var_dump(ini_set('session.hash_function', 'none')); // Should fail
var_dump(session_start());
var_dump(!empty(session_id()), session_id());
var_dump(session_destroy());


echo "Done";
ob_end_flush();
?>
