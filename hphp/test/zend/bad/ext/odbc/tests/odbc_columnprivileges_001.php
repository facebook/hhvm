<?php

include 'config.inc';

$conn = odbc_connect($dsn, $user, $pass);

var_dump($result = odbc_columnprivileges($conn, '', '', '', ''));
var_dump(odbc_fetch_row($result));

var_dump($result = odbc_columnprivileges($conn, NULL, NULL, NULL, NULL));
var_dump(odbc_fetch_row($result));

var_dump($result = odbc_columnprivileges($conn, 'FOO', 'FOO', 'FOO', 'FOO'));
var_dump(odbc_fetch_row($result));

?>
