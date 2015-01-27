<?php

ob_start();

/*
 * Prototype : void session_abort(void)
 * Description : Should abort session. Session data should not be written.
 * Source code : ext/session/session.c
 */

echo "*** Testing session_abort() : basic functionality ***\n";

session_start();
$session_id = session_id();
$_SESSION['foo'] = 123;
session_commit();

session_id($session_id);
session_start();
$_SESSION['bar'] = 456;
var_dump($_SESSION);
session_abort();

session_id($session_id);
session_start();
var_dump($_SESSION); // Should only have 'foo'

echo "Done".PHP_EOL;

?>
