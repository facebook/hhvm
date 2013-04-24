<?php
/* Prototype:  string stristr  ( string $haystack  , mixed $needle  [, bool $before_needle  ] )
   Description: Case-insensitive strstr().
*/

echo "*** Testing stristr() : basic functionality ***\n";

var_dump( stristr("Test string", "teSt") );
var_dump( stristr("test stRIng", "striNG") );
var_dump( stristr("teST StrinG", "stRIn") );
var_dump( stristr("tesT string", "t S") );
var_dump( stristr("test strinG", "g") );
var_dump( bin2hex(stristr(b"te".chr(0).b"St", chr(0))) );
var_dump( stristr("tEst", "test") );
var_dump( stristr("teSt", "test") );

var_dump( stristr("Test String", "String", false) );
var_dump( stristr("Test String", "String", true) );
?>
===DONE===