<?php
require 'connect.inc';
$db = new mysqli($host, $user, $passwd, 'mysql');

$stmt = $db->stmt_init();
$stmt->prepare("SELECT User FROM user WHERE password=\"\"");
$stmt->execute();
$stmt->bind_result($testArg);
echo "Okey";
?>