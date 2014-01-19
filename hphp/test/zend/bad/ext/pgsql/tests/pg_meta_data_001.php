<?php

include('config.inc');

$conn = pg_connect($conn_str);

pg_query('CREATE SCHEMA phptests');

pg_query('CREATE TABLE phptests.foo (id INT, id2 INT)');

pg_query('CREATE TABLE foo (id INT, id3 INT)');


var_dump(pg_meta_data($conn, 'foo'));
var_dump(pg_meta_data($conn, 'phptests.foo'));


pg_query('DROP TABLE foo');
pg_query('DROP TABLE phptests.foo');
pg_query('DROP SCHEMA phptests');

?>