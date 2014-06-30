<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table('field_flags'));
var_dump(mysql_query("insert into test_field_flags (name) values ('test'),('test2')"));

$res = mysql_query('select * from test_field_flags');
var_dump(mysql_field_flags($res, 0));
