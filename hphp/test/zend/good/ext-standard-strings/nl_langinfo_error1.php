<?php

/* Prototype  : string nl_langinfo  ( int $item  )
 * Description: Query language and locale information
 * Source code: ext/standard/string.c
*/

echo "*** Testing nl_langinfo() : error conditions ***\n";

echo "\n-- Testing nl_langinfo() function with no arguments --\n";
var_dump( nl_langinfo() );

echo "\n-- Testing nl_langinfo() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( nl_langinfo(ABDAY_2, $extra_arg) );

?>
===DONE===