<?php
/* Prototype  : string gzdeflate(string data [, int level])
 * Description: Gzip-compress a string 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

include(dirname(__FILE__) . '/data.inc');

echo "*** Testing gzdeflate() : variation ***\n";



echo "\n-- Testing multiple compression --\n";
$output = gzdeflate($data);
var_dump( md5($output));
var_dump(md5(gzdeflate($output)));

?>
===Done===