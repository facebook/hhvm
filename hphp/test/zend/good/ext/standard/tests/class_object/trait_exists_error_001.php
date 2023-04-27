<?hh
/* Prototype  : proto bool trait_exists(string traitname [, bool autoload])
 * Description: Checks if the trait exists 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

/**
 * Test wrong number of arguments
 */
<<__EntryPoint>> function trait_exists_error_001(): void {
echo "*** Testing trait_exists() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing trait_exists() function with Zero arguments --\n";
try { var_dump( trait_exists() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test trait_exists with one more than the expected number of arguments
echo "\n-- Testing trait_exists() function with more than expected no. of arguments --\n";
$traitname = 'string_val';
$autoload = true;
$extra_arg = 10;
try { var_dump( trait_exists($traitname, $autoload, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
