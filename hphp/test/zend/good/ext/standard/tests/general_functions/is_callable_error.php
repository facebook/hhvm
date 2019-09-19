<?hh
/* Prototype: bool is_callable ( mixed $var [, bool $syntax_only [, string &$callable_name]] );
   Description: Verify that the contents of a variable can be called as a function
                In case of objects, $var = array($SomeObject, 'MethodName')
*/
<<__EntryPoint>> function main(): void {
echo "\n*** Testing error conditions ***\n";

echo "\n-- Testing is_callable() function with less than expected no. of arguments --\n";
try { var_dump( is_callable() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing is_callable() function with more than expected no. of arguments --\n";
$callable_name = null;
try { var_dump( is_callable_with_name("string", TRUE, inout $callable_name, "EXTRA") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
