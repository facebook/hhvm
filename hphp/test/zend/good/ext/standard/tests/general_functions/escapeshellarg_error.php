<?hh
/* Prototype  : string escapeshellarg  ( string $arg  )
 * Description:  Escape a string to be used as a shell argument.
 * Source code: ext/standard/exec.c
 */
/*
 * Pass an incorrect number of arguments to escapeshellarg() to test behaviour
 */
class classA {}

<<__EntryPoint>> function main(): void {

echo "*** Testing escapeshellarg() : error conditions ***\n";

echo "\n-- Testing escapeshellarg() function with no arguments --\n";
try { var_dump( escapeshellarg() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing escapeshellarg() function with more than expected no. of arguments --\n";
$arg = "Mr O'Neil";
$extra_arg = 10;
try { var_dump( escapeshellarg($arg, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }





echo "\n-- Testing escapeshellarg() function with a object supplied for argument --\n";

$arg = new classA();
try { var_dump( escapeshellarg($arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing escapeshellarg() function with a resource supplied for argument --\n";
$fp = fopen(__FILE__, "r");
try { var_dump( escapeshellarg($fp)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
fclose($fp);

echo "\n-- Testing escapeshellarg() function with a array supplied for argument --\n";
$arg = vec[1,2,3];
try { var_dump( escapeshellarg($arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===Done===";
}
