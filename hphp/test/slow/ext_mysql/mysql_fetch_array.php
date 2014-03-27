<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table('fetch_array'));
var_dump(mysql_query("insert into test_fetch_array (name) values ('test'),('test2')"));

$res = mysql_query('select * from test_fetch_array');

$row = mysql_fetch_array($res);
print_r($row);
