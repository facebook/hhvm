<?php

echo "\n*** Testing error conditions ***\n";

echo "\n-- Testing ( (low < high) && (step = 0) ) --"; 
var_dump( range(1, 2, 0) );
var_dump( range("a", "b", 0) );

echo "\n\n-- Testing ( (low > high) && (step = 0) ) --";
var_dump( range(2, 1, 0) );
var_dump( range("b", "a", 0) );

echo "\n\n-- Testing ( (low < high) && (high-low < step) ) --";
var_dump( range(1.0, 7.0, 6.5) );

echo "\n\n-- Testing ( (low > high) && (low-high < step) ) --";
var_dump( range(7.0, 1.0, 6.5) );

echo "\n-- Testing Invalid number of arguments --";        
var_dump( range() );  // No.of args = 0
var_dump( range(1) );  // No.of args < expected
var_dump( range(1,2,3,4) );  // No.of args > expected
var_dump( range(-1, -2, 2) );  
var_dump( range("a", "j", "z") );

echo "\n-- Testing Invalid steps --";
$step_arr = array( "string", NULL, FALSE, "", "\0" );

foreach( $step_arr as $step ) {
  var_dump( range( 1, 5, $step ) );
}

echo "Done\n";
?>