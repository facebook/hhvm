<?php
session_start();
$_SESSION['x'] = "1\n";
echo $_SESSION['x'];

session_unset();
echo $_SESSION['x'];
echo "ok\n";
?>