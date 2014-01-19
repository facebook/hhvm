<?php

include 'config.inc';

// connect
$link = odbc_connect($dsn, $user, $pass)
  or die("Unable to connect");

// execute
$res = odbc_exec($link, "SELECT 1 as a, 2 as b, 3 as c")
  or die("Unable to execute SQL");

// check result
while ($row = odbc_fetch_array($res)) {
  var_dump($row);
}

odbc_close($link);
?>
