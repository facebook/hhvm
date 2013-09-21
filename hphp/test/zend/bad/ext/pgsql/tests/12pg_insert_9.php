<?php
error_reporting(E_ALL);

include 'config.inc';

$db = pg_connect($conn_str);
pg_query($db, "SET standard_conforming_strings = 0");

$fields = array('num'=>'1234', 'str'=>'AAA', 'bin'=>'BBB');

pg_insert($db, $table_name, $fields) or print "Error in test 1\n";
echo pg_insert($db, $table_name, $fields, PGSQL_DML_STRING)."\n";

echo "Ok\n";
?>