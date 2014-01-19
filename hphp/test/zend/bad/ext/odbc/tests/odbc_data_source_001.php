<?php

include 'config.inc';

$conn = odbc_connect($dsn, $user, $pass);

var_dump(odbc_data_source($conn, NULL));
var_dump(odbc_data_source($conn, ''));
var_dump(odbc_data_source($conn, SQL_FETCH_FIRST));

?>
