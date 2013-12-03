<?php
/* Prototype  : string gzcompress(string data [, int level, [int encoding]])
 * Description: Gzip-compress a string 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

include(dirname(__FILE__) . '/data.inc');

echo "*** Testing gzcompress() : variation ***\n";

echo "\n-- Testing multiple compression --\n";
$output = gzcompress($data);
var_dump( md5($output));
var_dump(md5(gzcompress($output)));

?>
===Done===