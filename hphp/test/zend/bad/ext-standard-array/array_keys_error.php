<?php

echo "\n*** Testing error conditions ***";
var_dump(array_keys(100));
var_dump(array_keys("string"));
var_dump(array_keys(new stdclass));  // object
var_dump(array_keys());  // Zero arguments
var_dump(array_keys(array(), "", TRUE, 100));  // args > expected
var_dump(array_keys(array(1,2,3, array() => array())));  // (W)illegal offset

echo "Done\n";
?>