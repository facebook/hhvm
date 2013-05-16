<?php

var_dump(json_encode("abc"));
var_dump(json_encode("ab\xE0"));
var_dump(json_encode("ab\xE0c"));
var_dump(json_encode(array("ab\xE0", "ab\xE0c", "abc")));

echo "Done\n";
?>