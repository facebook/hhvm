<?php

include 'config.inc';

$conn = odbc_connect($dsn, $user, $pass);

odbc_exec($conn, 'foo', 'bar');
odbc_exec($conn, 'foo');

odbc_exec($conn, '', '');
odbc_exec($conn, '');

odbc_exec($conn, 1, 1);
odbc_exec($conn, 1);

odbc_exec($conn, NULL, NULL);
odbc_exec($conn, NULL);

?>
