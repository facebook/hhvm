<?php
/* Prototype: string gettype ( mixed $var );
   Description: Returns the type of the PHP variable var

   Prototype: bool settype ( mixed &$var, string $type );
   Description: Set the type of variable var to type 
*/

/* Test different error conditions of settype() and gettype() functions */

echo "**** Testing gettype() and settype() functions ****\n";

echo "\n*** Testing gettype(): error conditions ***\n";
//Zero arguments
var_dump( gettype() );
// args more than expected 
var_dump( gettype( "1", "2" ) );

echo "\n*** Testing settype(): error conditions ***\n";
//Zero arguments
var_dump( settype() );

// args more than expected 
$var = 10.5;
var_dump( settype( $var, $var, "int" ) );

// passing an invalid type to set
var_dump( settype( $var, "unknown" ) );

echo "Done\n";
?>