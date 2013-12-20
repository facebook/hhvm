<?php

include 'config.inc';

// dumb calls
var_dump(odbc_connect("", "", ""));
var_dump(odbc_connect(NULL, NULL, NULL));

// connect
$link = odbc_connect($dsn, $user, $pass);
echo get_resource_type($link);

// and disconnect
odbc_close($link);
