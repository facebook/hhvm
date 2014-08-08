<?php

$var_name="SUCHVARSHOULDNOTEXIST";

var_dump(getenv($var_name));
var_dump(putenv($var_name."=value"));
var_dump(getenv($var_name));

var_dump(putenv($var_name."="));
var_dump(getenv($var_name));

var_dump(putenv($var_name));
var_dump(getenv($var_name));

var_dump(putenv("=123"));
var_dump(putenv(""));

echo "Done\n";
?>
