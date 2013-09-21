<?php

var_dump(filter_var("", FILTER_SANITIZE_STRING));
var_dump(filter_var("<>", FILTER_SANITIZE_STRING));
var_dump(filter_var("<>!@#$%^&*()'\"", FILTER_SANITIZE_STRING, FILTER_FLAG_NO_ENCODE_QUOTES));
var_dump(filter_var("<>!@#$%^&*()'\"", FILTER_SANITIZE_STRING, FILTER_FLAG_ENCODE_AMP));
var_dump(filter_var("<>`1234567890", FILTER_SANITIZE_STRING));
var_dump(filter_var("`123`", FILTER_SANITIZE_STRING));
var_dump(filter_var(".", FILTER_SANITIZE_STRING));

echo "Done\n";
?>