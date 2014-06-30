<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
var_dump(create_test_table('lengths'));
var_dump(mysql_query("insert into test_lengths (name) values ('test'),('test2')"));

$res = mysql_query('select * from test_lengths');
$row = mysql_fetch_row($res);
$lengths = mysql_fetch_lengths($res);

print_r($lengths);

// A much more intense test on lengths
mysql_query('drop table testlen');
var_dump(mysql_query("create table testlen (id int not null auto_increment, " .
                   "d decimal(10,5), t tinyint, i int, b bigint, f float, " .
                   "db double, y2 year(2), y4 year(4), primary key (id)) " .
                   "engine=innodb"));
var_dump(mysql_query("insert into testlen(d, t, i, b, f, db, y2, y4) values" .
                   "(.343, null, 384, -1, 03.44, -03.43892874e101, 00, 0000)"));

$res = mysql_query('select * from testlen');
$row = mysql_fetch_row($res);
$lengths = mysql_fetch_lengths($res);

print_r($lengths);
