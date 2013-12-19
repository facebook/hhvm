<?php
ini_set('error_reporting ',  E_ALL & ~E_NOTICE | E_STRICT);

ini_set('precision', 14);

/* Prototype: bool is_callable ( mixed $var [, bool $syntax_only [, string &$callable_name]] );
   Description: Verify that the contents of a variable can be called as a function
                In case of objects, $var = array($SomeObject, 'MethodName')
*/

echo "\n*** Testing error conditions ***\n";

echo "\n-- Testing is_callable() function with less than expected no. of arguments --\n";
var_dump( is_callable() );

echo "\n-- Testing is_callable() function with more than expected no. of arguments --\n";
var_dump( is_callable("string", TRUE, $callable_name, "EXTRA") );

?>
===DONE===