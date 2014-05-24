<?php

ob_start();

/* 
 * Prototype : bool session_start(void)
 * Description : Initialize session data
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_start() : variation ***\n";

$_SESSION['blah'] = 'foo';
var_dump($_SESSION);
session_start();
var_dump($_SESSION);

session_destroy();
echo "Done";
ob_end_flush();

?>
