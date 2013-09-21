<?php
ob_start();

ini_set('session.serialize_handler', 'php_serialize');
var_dump(session_start());
$_SESSION[-3] = 'foo';
$_SESSION[3] = 'bar';
$_SESSION['var'] = 123;
var_dump(session_encode());
session_write_close();

// Should finish without errors
echo 'Done'.PHP_EOL;
?>