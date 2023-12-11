<?hh
/* Prototype  :  array getrusage  ([ int $who  ] )
 * Description: Gets the current resource usages
 * Source code: ext/standard/microtime.c
 * Alias to functions:
 */
/*
 * Pass an incorrect number of arguments to getrusage() to test behaviour
 */
class classA { function __toString() :mixed{ return "ClassAObject"; } }
<<__EntryPoint>> function main(): void {
echo "*** Testing getrusage() : error conditions ***\n";

echo "\n-- Testing getrusage() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { $dat = getrusage(1, $extra_arg); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing getrusage() function with invalid argument - non-numeric STRING--\n";
$string_arg = "foo";
try { $dat = getrusage($string_arg); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing getrusage() function with invalid argument - ARRAY--\n";
$array_arg = vec[1,2,3];
try { $dat = getrusage($array_arg); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing getrusage() function with invalid argument - OBJECT --\n";






$obj_arg = new classA();
try { $dat = getrusage($obj_arg); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing getrusage() function with invalid argument - RESOURCE --\n";
$file_handle=fopen(__FILE__, "r");
try { $dat = getrusage($file_handle); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
fclose($file_handle);

echo "===DONE===\n";
}
