<?php
$invalid_utf8 = "\x9f";

var_dump(json_encode($invalid_utf8));
var_dump(json_last_error(), json_last_error_msg());

var_dump(json_encode($invalid_utf8, JSON_PARTIAL_OUTPUT_ON_ERROR));
var_dump(json_last_error(), json_last_error_msg());

echo "\n";

$invalid_utf8 = "an invalid sequen\xce in the middle of a string";

var_dump(json_encode($invalid_utf8));
var_dump(json_last_error(), json_last_error_msg());

var_dump(json_encode($invalid_utf8, JSON_PARTIAL_OUTPUT_ON_ERROR));
var_dump(json_last_error(), json_last_error_msg());

?>