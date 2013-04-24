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
octdec();
octdec('0123567',true);

echo "\n-- Incorrect input --\n";
octdec(new classA());


?>