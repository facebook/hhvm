<?hh
/* Prototype  : bool interface_exists(string classname [, bool autoload])
 * Description: Checks if the class exists 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */
<<__EntryPoint>> function interface_exists_error(): void {
echo "*** Testing interface_exists() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing interface_exists() function with Zero arguments --\n";
try { var_dump( interface_exists() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test interface_exists with one more than the expected number of arguments
echo "\n-- Testing interface_exists() function with more than expected no. of arguments --\n";
$classname = 'string_val';
$autoload = true;
$extra_arg = 10;
try { var_dump( interface_exists($classname, $autoload, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
