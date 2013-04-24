<?php

include('config.inc');

$conn = pg_connect($conn_str);

pg_query('CREATE SCHEMA phptests');

pg_query('CREATE TABLE foo (id INT, id2 INT)');
pg_query('CREATE TABLE phptests.foo (id INT, id2 INT)');


pg_insert($conn, 'foo', array('id' => 1, 'id2' => 1));
pg_insert($conn, 'phptests.foo', array('id' => 1, 'id2' => 2));

pg_update($conn, 'foo', array('id' => 10), array('id' => 1));
var_dump(pg_update($conn, 'foo', array('id' => 10), array('id' => 1), PGSQL_DML_STRING));

pg_update($conn, 'phptests.foo', array('id' => 100), array('id2' => 2));
var_dump(pg_update($conn, 'phptests.foo', array('id' => 100), array('id2' => 2), PGSQL_DML_STRING));

$rs = pg_query('SELECT * FROM foo UNION SELECT * FROM phptests.foo');
while ($row = pg_fetch_assoc($rs)) {
	var_dump($row);
}

pg_query('DROP TABLE foo');
pg_query('DROP TABLE phptests.foo');
pg_query('DROP SCHEMA phptests');

?>