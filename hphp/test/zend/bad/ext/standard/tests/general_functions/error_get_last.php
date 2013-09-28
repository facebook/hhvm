<?php

var_dump(error_get_last());
var_dump(error_get_last(true));
var_dump(error_get_last());

$a = $b;

var_dump(error_get_last());

echo "Done\n";
?>