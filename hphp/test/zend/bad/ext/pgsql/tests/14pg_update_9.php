<?php
error_reporting(E_ALL);

include 'config.inc';

$db = pg_connect($conn_str);
pg_query($db, "SET standard_conforming_strings = 0");

$fields = array('num'=>'1234', 'str'=>'ABC', 'bin'=>'XYZ');
$ids = array('num'=>'1234');

pg_update($db, $table_name, $fields, $ids) or print "Error in test 1\n";
echo pg_update($db, $table_name, $fields, $ids, PGSQL_DML_STRING)."\n";

echo "Ok\n";
?>