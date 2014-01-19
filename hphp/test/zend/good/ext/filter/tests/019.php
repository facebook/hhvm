<?php

var_dump(filter_var("....", FILTER_VALIDATE_IP));
var_dump(filter_var("...", FILTER_VALIDATE_IP));
var_dump(filter_var("..", FILTER_VALIDATE_IP));
var_dump(filter_var(".", FILTER_VALIDATE_IP));
var_dump(filter_var("1.1.1.1", FILTER_VALIDATE_IP));

echo "Done\n";
?>