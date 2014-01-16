<?php
/* include('test.inc'); */
echo "iconv extension is available\n";
$test = "זרו";
var_dump("ISO-8859-1: $test");
var_dump("UTF-8: ".iconv( "ISO-8859-1", "UTF-8", $test ) );
?>