<?php
/* Prototype  : string gzencode  ( string $data  [, int $level  [, int $encoding_mode  ]] )
 * Description: Gzip-compress a string 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

include(dirname(__FILE__) . '/data.inc');

echo "*** Testing gzencode() : variation ***\n";

echo "\n-- Testing multiple compression --\n";
$output = gzencode($data);
var_dump(bin2hex(gzencode($output)));

?>
===Done===