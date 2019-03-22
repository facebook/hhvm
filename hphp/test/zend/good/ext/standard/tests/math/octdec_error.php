<?php
/* Prototype  : number octdec  ( string $octal_string  )
 * Description: Returns the decimal equivalent of the octal number represented by the octal_string  argument. 
 * Source code: ext/standard/math.c
 */

echo "*** Testing octdec() :  error conditions ***\n";

// get a class
class classA
{
}

echo "\n-- Incorrect number of arguments --\n"; 
try { octdec(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { octdec('0123567',true); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Incorrect input --\n";
octdec(new classA());


