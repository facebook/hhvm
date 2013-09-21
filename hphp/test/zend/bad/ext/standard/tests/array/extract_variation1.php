<?php

$val = 4;
$str = "John";

debug_zval_dump($val);
debug_zval_dump($str);

/* Extracting Global Variables */
var_dump(extract($GLOBALS, EXTR_REFS));
debug_zval_dump($val);
debug_zval_dump($str);

echo "\nDone";
?>
