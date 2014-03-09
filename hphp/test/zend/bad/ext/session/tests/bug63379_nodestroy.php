<?php

ob_start();

$handler = new SessionHandler;
session_set_save_handler($handler);

session_start();

$_SESSION['foo'] = 'hello';
var_dump($_SESSION);

session_regenerate_id(false);

echo "*** Regenerated ***\n";
var_dump($_SESSION);

$_SESSION['bar'] = 'world';

var_dump($_SESSION);

session_write_close();
session_unset();

session_start();
var_dump($_SESSION);
