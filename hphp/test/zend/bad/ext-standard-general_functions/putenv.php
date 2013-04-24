<?php

$var_name="SUCHVARSHOULDNOTEXIST";

var_dump(getenv($var_name));
var_dump(putenv($var_name."=value"));
var_dump(getenv($var_name));

var_dump(putenv($var_name."="));
var_dump(getenv($var_name));

var_dump(putenv($var_name));
var_dump(getenv($var_name));

echo "Done\n";
?>