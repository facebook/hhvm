<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table('fetch_row'));
var_dump(mysql_query("insert into test_fetch_row (name) values ('test'),('test2')"));

$res = mysql_query('select * from test_fetch_row');

$row = mysql_fetch_row($res);
print_r($row);
