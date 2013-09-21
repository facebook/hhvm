<?php

var_dump(json_encode("abc"));
var_dump(json_encode("ab\xE0"));
var_dump(json_encode("ab\xE0", JSON_PARTIAL_OUTPUT_ON_ERROR));
var_dump(json_encode(array("ab\xE0", "ab\xE0c", "abc"), JSON_PARTIAL_OUTPUT_ON_ERROR));

echo "Done\n";
?>