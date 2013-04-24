<?php

include('config.inc');

$conn = pg_connect($conn_str);

pg_query('CREATE SCHEMA phptests');

pg_query('CREATE TABLE phptests.foo (id INT, id2 INT)');
pg_query('INSERT INTO phptests.foo VALUES (1,2)');
pg_query('INSERT INTO phptests.foo VALUES (2,3)');

pg_query('CREATE TABLE phptests.bar (id4 INT, id3 INT)');
pg_query('INSERT INTO phptests.bar VALUES (4,5)');
pg_query('INSERT INTO phptests.bar VALUES (6,7)');

/* Inexistent table */
var_dump(pg_select($conn, 'foo', array('id' => 1)));

/* Existent column */
var_dump(pg_select($conn, 'phptests.foo', array('id' => 1)));

/* Testing with inexistent column */
var_dump(pg_select($conn, 'phptests.bar', array('id' => 1)));

/* Existent column */
var_dump(pg_select($conn, 'phptests.bar', array('id4' => 4)));


pg_query('DROP TABLE phptests.foo');
pg_query('DROP TABLE phptests.bar');
pg_query('DROP SCHEMA phptests');

?>