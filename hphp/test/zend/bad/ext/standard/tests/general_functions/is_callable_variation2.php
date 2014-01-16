<?php
ini_set('error_reporting ',  E_ALL & ~E_NOTICE | E_STRICT);

ini_set('precision', 14);

/* Prototype: bool is_callable ( mixed $var [, bool $syntax_only [, string &$callable_name]] );
   Description: Verify that the contents of a variable can be called as a function
                In case of objects, $var = array($SomeObject, 'MethodName')
*/

/* Prototype: void check_iscallable( $functions );
   Description: use iscallable() on given string to check for valid function name
                returns true if valid function name, false otherwise
*/
function check_iscallable( $functions ) {
  $counter = 1;
  foreach($functions as $func) {
    echo "-- Iteration  $counter --\n";
    var_dump( is_callable($func) );  //given only $var argument
    var_dump( is_callable($func, TRUE) );  //given $var and $syntax argument
    var_dump( is_callable($func, TRUE, $callable_name) );
    echo $callable_name, "\n";
    var_dump( is_callable($func, FALSE) );  //given $var and $syntax argument
    var_dump( is_callable($func, FALSE, $callable_name) );
    echo $callable_name, "\n";
    $counter++;
  }
}

echo "\n*** Testing is_callable() on invalid function names ***\n";
/* check on unset variables */
$unset_var = 10;
unset ($unset_var);

/* opening file resource type */
$file_handle = fopen (__FILE__, "r");

$variants = array (
  NULL,  // NULL as argument
  0,  // zero as argument
  1234567890,  // positive value
  -100123456782,  // negative value
  -2.000000,  // negative float value
  .567,  // positive float value
  FALSE,  // boolean value
  array(1, 2, 3),  // array
  @$unset_var,
  @$undef_var,  //undefined variable
  $file_handle
);

/* use check_iscallable() to check whether given variable is valid function name
 *  expected: false
 */
check_iscallable($variants);

/* closing resources used */
fclose($file_handle);

?>
===DONE===