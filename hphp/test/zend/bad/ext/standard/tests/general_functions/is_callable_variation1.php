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

echo "\n*** Testing is_callable() on undefined functions ***\n";
$undef_functions = array (
  "",  //empty string
  '',
  " ",  //string with a space
  ' ',
  "12356",
  "\0",
  '\0',
  "hello world",
  'hello world',
  "welcome\0",
  'welcome\0',
  "==%%%***$$$@@@!!",
  "false",
  "\070",
  '\t',  //escape character
  '\007',
  '123',
  'echo()'
);

/* use check_iscallable() to check whether given string is valid function name
 * expected: true with $syntax = TRUE
 *           false with $syntax = FALSE
 */
check_iscallable($undef_functions);

?>
===DONE===