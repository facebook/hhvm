<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
$dbs = mysql_list_dbs();
var_dump((bool)mysql_db_name($dbs, 0));
