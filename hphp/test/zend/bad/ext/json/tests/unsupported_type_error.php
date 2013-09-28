<?php

$resource = fopen(__FILE__, "r");

var_dump($resource);

var_dump(json_encode($resource));
var_dump(json_last_error(), json_last_error_msg());

var_dump(json_encode($resource, JSON_PARTIAL_OUTPUT_ON_ERROR));
var_dump(json_last_error(), json_last_error_msg());

?>