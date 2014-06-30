<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table('field_name'));
var_dump(mysql_query("insert into test_field_name (name) values ('test'),('test2')"));

$res = mysql_query('select * from test_field_name');
var_dump(mysql_field_name($res, 1));
