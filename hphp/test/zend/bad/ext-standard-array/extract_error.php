<?php

/* Testing Error Conditions */
echo "*** Testing Error Conditions ***\n";

/* Zero Arguments */
var_dump( extract() );

/* Invalid second argument ( only 0-6 is valid) */
$arr = array(1);
var_dump( extract($arr, -1 . "wddr") );
var_dump( extract($arr, 7 , "wddr") );

/* scalar argument */
$val = 1;
var_dump( extract($val) );

/* string argument */
$str = "test";
var_dump( extract($str) );

/* More than valid number of arguments i.e. 3 args */
var_dump( extract($arr, EXTR_SKIP, "aa", "ee") );

/* Two Arguments, second as prefix but without prefix string as third argument */
var_dump( extract($arr,EXTR_PREFIX_IF_EXISTS) );

echo "Done\n";
?>
