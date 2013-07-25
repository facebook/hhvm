<?php
error_reporting(E_ALL);

include 'config.inc';

$db = pg_connect($conn_str);
pg_query($db, "SET standard_conforming_strings = 0");

$fields = array('num'=>'1234', 'str'=>'AAA', 'bin'=>'BBB');
$converted = pg_convert($db, $table_name, $fields);

var_dump($converted);
?>