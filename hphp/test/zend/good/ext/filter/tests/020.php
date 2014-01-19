<?php

var_dump(filter_var("test'asd'asd'' asd\'\"asdfasdf", FILTER_SANITIZE_MAGIC_QUOTES));
var_dump(filter_var("'", FILTER_SANITIZE_MAGIC_QUOTES));
var_dump(filter_var("", FILTER_SANITIZE_MAGIC_QUOTES));
var_dump(filter_var(-1, FILTER_SANITIZE_MAGIC_QUOTES));

echo "Done\n";
?>