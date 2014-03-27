<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
$item = "Zak's Laptop";
var_dump(mysql_real_escape_string($item));
mysql_close($conn);
var_dump(@mysql_real_escape_string($item));
