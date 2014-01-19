<?php

include 'config.inc';

$conn = odbc_connect($dsn, $user, $pass);

odbc_exec($conn, 'CREATE DATABASE odbcTEST');

odbc_exec($conn, 'CREATE TABLE FOO (TEST INT)');
odbc_exec($conn, 'ALTER TABLE FOO ADD PRIMARY KEY FOO(TEST)');

odbc_exec($conn, 'INSERT INTO FOO VALUES (1)');
odbc_exec($conn, 'INSERT INTO FOO VALUES (2)');

$res = odbc_exec($conn, 'SELECT * FROM FOO');

var_dump(odbc_fetch_row($res));
var_dump(odbc_result($res, 'test'));
var_dump(odbc_free_result($res));
var_dump(odbc_free_result($conn));
var_dump(odbc_free_result(NULL));
var_dump(odbc_fetch_row($res));
var_dump(odbc_result($res, 'test'));

odbc_exec($conn, 'DROP TABLE FOO');

odbc_exec($conn, 'DROP DATABASE odbcTEST');

?>
