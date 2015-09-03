<?php
/*
 * This basically copies zend/ext/spl/tests/SplFixedArray_setSize_param_null.php
 * but changes the expectf to match hhvm's var_dump output
*/

$fixed_array = new SplFixedArray(2);
$fixed_array->setSize(null);
var_dump($fixed_array);
