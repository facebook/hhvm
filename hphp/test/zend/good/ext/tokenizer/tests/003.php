<?php

var_dump(token_get_all(array()));
var_dump(token_get_all(new stdClass));
var_dump(token_get_all(""));
var_dump(token_get_all(0));
var_dump(token_get_all(-1));

echo "Done\n";
?>
