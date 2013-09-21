<?php

var_dump(filter_var("\"<br>blah</ph>", FILTER_SANITIZE_ENCODED));
var_dump(filter_var("", FILTER_SANITIZE_ENCODED));
var_dump(filter_var("  text here  ", FILTER_SANITIZE_ENCODED));
var_dump(filter_var("!@#$%^&*()QWERTYUIOP{ASDFGHJKL:\"ZXCVBNM<>?", FILTER_SANITIZE_ENCODED));

echo "Done\n";
?>