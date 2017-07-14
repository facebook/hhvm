<?php
var_dump(assert(false));
var_dump(assert(true));
ini_set("zend.assertions", 0);
ini_set("zend.assertions", 1);
?>
