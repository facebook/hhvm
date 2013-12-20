<?php

include 'config.inc';

// connect
$link = odbc_connect($dsn, $user, $pass)
  or die("Unable to connect");

// disable
odbc_autocommit($link, false);

// check behavior
var_dump(odbc_autocommit($link));

// enable
odbc_autocommit($link, true);

// check behavior
var_dump(odbc_autocommit($link));

odbc_close($link);
?>
