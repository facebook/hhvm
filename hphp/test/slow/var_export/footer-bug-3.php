<?php
$obj = new stdClass();
$obj->prop = imagecreate(10, 10);
$str = var_export($obj, true);
// HHVM has a bug with var_export() where resources are converted to
// "array()" instead of NULL. We do this substitution here so that this
// test's expected output is not affected by this bug.
$str = strtr($str, array("\n  array (\n  )" => "NULL"));
echo $str, "\nDone\n";
