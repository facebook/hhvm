<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table('num_fields'));
var_dump(mysql_query("INSERT INTO test_num_fields (name) VALUES ('test'),('test2')"));

$res = mysql_query('select * from test_num_fields');
var_dump(mysql_num_fields($res));
