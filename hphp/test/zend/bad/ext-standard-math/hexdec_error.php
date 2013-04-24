<?php
/* Prototype  : number hexdec  ( string $hex_string  )
 * Description: Returns the decimal equivalent of the hexadecimal number represented by the hex_string  argument. 
 * Source code: ext/standard/math.c
 */

echo "*** Testing hexdec() :  error conditions ***\n";

// get a class
class classA
{
}

echo "\n-- Incorrect number of arguments --\n";
hexdec();
hexdec('0x123abc',true);

echo "\n-- Incorrect input --\n";
hexdec(new classA());

?>