<?php
require 'connect.inc';
if (!$db = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
	printf("[001] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
}

$stmt = $db->stmt_init();
$stmt->prepare("SELECT User FROM user WHERE password=\"\"");
$stmt->execute();
$stmt->bind_result($testArg);
echo "Okey";
?>
