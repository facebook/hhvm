<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
$res = mysql_list_processes();
$process = mysql_fetch_assoc($res);
var_dump(!empty($process['Id']));
