<?php

require_once('config.inc');
	
$dbh = @pg_connect($conn_str);
if (!$dbh) {
	die ("Could not connect to the server");
}

pg_query("CREATE TABLE php_test (id SERIAL PRIMARY KEY, time TIMESTAMP NOT NULL DEFAULT now())");

pg_insert($dbh, 'php_test', array());

var_dump(pg_fetch_assoc(pg_query("SELECT * FROM php_test")));

pg_query($dbh, "DROP TABLE php_test");
pg_close($dbh);
?>
===DONE===