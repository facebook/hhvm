<?php
/* Prototype  : string mb_internal_encoding([string $encoding])
 * Description: Sets the current internal encoding or 
 * Returns the current internal encoding as a string 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass mb_internal_encoding an unknown encoding
 */

echo "*** Testing mb_internal_encoding() : error conditions ***\n";

var_dump(mb_internal_encoding('unknown-encoding'));

echo "Done";
?>