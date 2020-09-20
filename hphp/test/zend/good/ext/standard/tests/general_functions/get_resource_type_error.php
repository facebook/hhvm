<?hh
/* Prototype  : string get_resource_type  ( resource $handle  )
 * Description:  Returns the resource type
 * Source code: Zend/zend_builtin_functions.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing get_resource_type() : error conditions ***\n";

echo "\n-- Testing get_resource_type() function with Zero arguments --\n";
try { var_dump( get_resource_type() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing get_resource_type() function with more than expected no. of arguments --\n";
$res = fopen(__FILE__, "r");
$extra_arg = 10;
try { var_dump( get_resource_type($res, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
