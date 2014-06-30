<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table('data_seek'));
var_dump(mysql_query("insert into test_data_seek (name) values ('test'),('test2')"));

$res = mysql_query('select * from test_data_seek');
var_dump(mysql_data_seek($res, 1));

$row = mysql_fetch_assoc($res);
print_r($row);
