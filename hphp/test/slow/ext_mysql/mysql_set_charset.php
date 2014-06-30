<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
var_dump(mysql_set_charset('utf8', $conn));
