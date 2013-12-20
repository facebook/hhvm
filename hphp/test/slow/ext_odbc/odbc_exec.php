<?php

include 'config.inc';


// raise a warning
odbc_exec(NULL, NULL);

// connect
$link = odbc_connect($dsn, $user, $pass)
  or die("Unable to connect");

// this should raise one more warning
$res = odbc_exec($link, "");

// invalid query, another warning
$res = odbc_exec($link, "notasqlstatement");

// now a valid query
$res = odbc_exec($link, "SELECT 1");
echo get_resource_type($res);

// and disconnect
odbc_close($link);

?>
