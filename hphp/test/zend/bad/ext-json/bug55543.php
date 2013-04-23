<?php
$a = new stdClass;
$a->{"1"} = "5";

var_dump(json_encode($a, JSON_NUMERIC_CHECK));
?>