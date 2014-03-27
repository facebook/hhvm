<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table('free_result'));

$res = mysql_query('select * from test_free_result');
var_dump(mysql_free_result($res));
