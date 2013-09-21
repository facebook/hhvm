<?php
/* Prototype  : string gzencode  ( string $data  [, int $level  [, int $encoding_mode  ]] )
 * Description: Gzip-compress a string 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */

echo "*** Testing gzencode() : variation ***\n";

$data = "A small string to encode\n";

echo "\n-- Testing with each encoding_mode  --\n";
var_dump(bin2hex(gzencode($data, -1)));
var_dump(bin2hex(gzencode($data, -1, FORCE_GZIP)));  
var_dump(bin2hex(gzencode($data, -1, FORCE_DEFLATE)));

?>
===DONE===