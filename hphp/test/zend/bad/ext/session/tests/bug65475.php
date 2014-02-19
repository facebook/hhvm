<?php
ob_start();

echo "Testing file module".PHP_EOL;
session_start();
$_SESSION['foo'] = 1234;
$_SESSION['cnt'] = 1;
$session_id = session_id();
session_write_close();

session_start();
var_dump($session_id === session_id());
$_SESSION['cnt']++;
session_write_close();

session_start();
var_dump($session_id === session_id());
var_dump($_SESSION['cnt']); // Should be int(2)
session_write_close();
