<?php

include 'config.inc';

$db = pg_connect($conn_str);
$result = pg_query("select 'a' union select 'b'");

var_dump(pg_fetch_array($result, -1));
var_dump(pg_fetch_assoc($result, -1));
var_dump(pg_fetch_object($result, -1));
var_dump(pg_fetch_row($result, -1));

var_dump(pg_fetch_array($result, 0));
var_dump(pg_fetch_assoc($result, 0));
var_dump(pg_fetch_object($result, 0));
var_dump(pg_fetch_row($result, 0));

pg_close($db);

?>