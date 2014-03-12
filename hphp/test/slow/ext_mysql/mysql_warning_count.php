<?php
require_once('connect.inc');

$conn = mysql_connect($host, $user, $passwd);
// No warnings from normal operation
var_dump(create_test_table('warning'));
var_dump(mysql_warning_count($conn));
var_dump(mysql_query("INSERT INTO test_warning (name) VALUES ('test'),('test2')"));
var_dump(mysql_warning_count($conn));

// Dropping a non-existent table with IF EXISTS generates a warning.
var_dump(mysql_query("DROP TABLE IF EXISTS no_such_table"));
var_dump(mysql_warning_count($conn));

// Dropping an existing table generates no warnings.
var_dump(mysql_query("DROP TABLE IF EXISTS test_warning"));
var_dump(mysql_warning_count($conn));
