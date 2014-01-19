<?php

require_once('config.inc');
	
$dbh = @pg_connect($conn_str);
if (!$dbh) {
	die ("Could not connect to the server");
}

pg_query("CREATE TABLE php_test (id SERIAL, tm timestamp NOT NULL)");

$values = array('tm' => 'now()');
pg_insert($dbh, 'php_test', $values);

$ids = array('id' => 1);
pg_update($dbh, 'php_test', $values, $ids);

pg_query($dbh, "DROP TABLE php_test");
pg_close($dbh);
?>
===DONE===