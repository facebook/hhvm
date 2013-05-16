<?php

ob_start();

/* 
 * Prototype : bool session_start(void)
 * Description : Initialize session data
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_start() : variation ***\n";

session_start();

$_SESSION['colour'] = 'green';
$_SESSION['animal'] = 'cat';
$_SESSION['person'] = 'julia';
$_SESSION['age'] = 6;

var_dump($_SESSION);
var_dump(session_write_close());
var_dump($_SESSION);

session_start();
session_destroy();
echo "Done";
ob_end_flush();
?>