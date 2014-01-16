<?php
ini_set('precision', 14);


var_dump(hexdec("012345"));
var_dump(hexdec("12345"));
var_dump(hexdec("q12345"));
var_dump(hexdec("12345+?!"));
var_dump(hexdec("12345q"));
var_dump((float)hexdec("1234500001"));
var_dump((float)hexdec("17fffffff"));

?>