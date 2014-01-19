<?php

include 'config.inc';

// connect
$link = odbc_connect($dsn, $user, $pass)
  or die("Unable to connect");

// create tmp table
odbc_exec($link, "CREATE TABLE phpodbc_test (id INTEGER)");

// prepare stmt
$stmt = odbc_prepare($link, "INSERT INTO phpodbc_test VALUES ( ? )")
  or die("Unable to create stmt " . odbc_errormsg());

// check return
echo get_resource_type($stmt) . "\n";

// execute stmt
odbc_execute($stmt, array(array(1, 2, 3)))
  or die("Unable to create stmt");

// retrieve data
$res = odbc_exec($link, "SELECT * FROM phpodbc_test ORDER BY id");

// check retrieved data
while ($row = odbc_fetch_array($res)) {
  var_dump($row);
}

// drop tmp table
odbc_exec($link, "DROP TABLE phpodbc_test");

odbc_close($link);
?>
