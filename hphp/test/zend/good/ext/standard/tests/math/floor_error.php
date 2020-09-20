<?hh
/* Prototype  : float floor  ( float $value  )
 * Description: Round fractions down.
 * Source code: ext/standard/math.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing floor() :  error conditions ***\n";
$arg_0 = 1.0;
$extra_arg = 1;

echo "\n-- Too many arguments --\n";
try { var_dump(floor($arg_0, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Too few arguments --\n";
try { var_dump(floor()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===Done===";
}
