<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
$tables = mysql_list_tables($db);
var_dump((bool)mysql_tablename($tables, 0));
