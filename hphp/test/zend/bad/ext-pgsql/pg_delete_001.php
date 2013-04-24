<?php

include('config.inc');

$conn = pg_connect($conn_str);

pg_query('CREATE SCHEMA phptests');

pg_query('CREATE TABLE foo (id INT, id2 INT)');
pg_query('CREATE TABLE phptests.foo (id INT, id2 INT)');

pg_insert($conn, 'foo', array('id' => 1, 'id2' => 1));
pg_insert($conn, 'foo', array('id' => 1, 'id2' => 2));
pg_insert($conn, 'foo', array('id' => 1, 'id2' => 2));
pg_insert($conn, 'foo', array('id' => 3, 'id2' => 3));

pg_insert($conn, 'phptests.foo', array('id' => 1, 'id2' => 1));
pg_insert($conn, 'phptests.foo', array('id' => 1, 'id2' => 2));
pg_insert($conn, 'phptests.foo', array('id' => 2, 'id2' => 3));
pg_insert($conn, 'phptests.foo', array('id' => 2, 'id2' => 3));

pg_delete($conn, 'foo', array('id' => 1, 'id2' => 0));
pg_delete($conn, 'foo', array('id' => 1, 'id2' => 2));
var_dump(pg_delete($conn, 'foo', array('id' => 1, 'id2' => 2), PGSQL_DML_STRING));

pg_delete($conn, 'phptests.foo', array('id' => 2, 'id2' => 1));
pg_delete($conn, 'phptests.foo', array('id' => 2, 'id2' => 3));
var_dump(pg_delete($conn, 'phptests.foo', array('id' => 2, 'id2' => 3), PGSQL_DML_STRING));

var_dump(pg_fetch_all(pg_query('SELECT * FROM foo')));
var_dump(pg_fetch_all(pg_query('SELECT * FROM phptests.foo')));

/* Inexistent */
pg_delete($conn, 'bar', array('id' => 1, 'id2' => 2));
var_dump(pg_delete($conn, 'bar', array('id' => 1, 'id2' => 2), PGSQL_DML_STRING));

pg_query('DROP TABLE foo');
pg_query('DROP TABLE phptests.foo');
pg_query('DROP SCHEMA phptests');

?>