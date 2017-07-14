<?php
var_dump((integer)ini_get("zend.assertions"));
ini_set("zend.assertions", 0);
var_dump((integer)ini_get("zend.assertions"));
assert(false);
ini_set("zend.assertions", 1);
var_dump((integer)ini_get("zend.assertions"));
assert(true);
var_dump(true);
?>
