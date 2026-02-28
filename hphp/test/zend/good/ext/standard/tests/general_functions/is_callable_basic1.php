<?hh
/* Prototype: bool is_callable ( mixed $var [, bool $syntax_only [, string &$callable_name]] );
 * Description: Verify that the contents of a variable can be called as a function
 * Source code: ext/imap/php_imap.c
 */

/* Prototype: void check_iscallable( $functions );
   Description: use iscallable() on given string to check for valid function name
                returns true if valid function name, false otherwise
*/
function check_iscallable( $functions ) :mixed{
  $counter = 1;
  foreach($functions as $func) {
    echo "-- Iteration  $counter --\n";
    var_dump( is_callable($func) );  //given only $var argument
    var_dump( is_callable($func, TRUE) );  //given $var and $syntax argument
    $callable_name = null;
    var_dump( is_callable_with_name($func, TRUE, inout $callable_name) );
    echo $callable_name, "\n";
    var_dump( is_callable($func, FALSE) );  //given $var and $syntax argument
    var_dump( is_callable_with_name($func, FALSE, inout $callable_name) );
    echo $callable_name, "\n";
    $counter++;
  }
}

/* function name with simple string */
function someFunction() :mixed{
}

/* function name with mixed string and integer */
function x123() :mixed{
}

/* function name as NULL */
function NULL() :mixed{
}

/* function name with boolean name */
function false() :mixed{
}

/* function name with string and special character */
function Hello_World() :mixed{
}
<<__EntryPoint>> function main(): void {
echo "\n*** Testing is_callable() on defined functions ***\n";
$defined_functions = vec[
  $functionVar1 = 'someFunction',
  $functionVar2 = 'x123',
  $functionVar3 = 'NULL',
  $functionVar4 = 'false',
  $functionVar5 = "Hello_World"
];
/* use check_iscallable() to check whether given string is valid function name
 *  expected: true as it is valid callback
 */
check_iscallable($defined_functions);

echo "===DONE===\n";
}
