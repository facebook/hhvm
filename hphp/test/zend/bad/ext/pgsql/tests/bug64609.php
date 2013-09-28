<?php
error_reporting(E_ALL);

include 'config.inc';

$db = pg_connect($conn_str);
pg_query("BEGIN");
pg_query("CREATE TYPE t_enum AS ENUM ('ok', 'ko')");
pg_query("CREATE TABLE test_enum (a t_enum)");

$fields = array('a' => 'ok');
$converted = pg_convert($db, 'test_enum', $fields);

pg_query("ROLLBACK");

var_dump($converted);
?>