<?hh
/* Prototype  : array gd_info()
 * Description: Retrieve information about the currently installed GD library
 * Source code: ext/gd/gd.c  */
<<__EntryPoint>> function main(): void {
$extra_arg_number = 10;
$extra_arg_string = "Hello";

echo "*** Testing gd_info() : error conditions ***\n";

echo "\n-- Testing gd_info() function with more than expected number of arguments --\n";
try { var_dump(gd_info($extra_arg_number)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(gd_info($extra_arg_string, $extra_arg_number)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
