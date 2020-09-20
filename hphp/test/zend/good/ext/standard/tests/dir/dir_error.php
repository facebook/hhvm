<?hh
/* 
 * Prototype  : object dir(string $directory[, resource $context])
 * Description: Directory class with properties, handle and class and methods read, rewind and close
 * Source code: ext/standard/dir.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing dir() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing dir() function with zero arguments --";
try { var_dump( dir() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// With one more than expected number of arguments
echo "\n-- Testing dir() function with one more than expected number of arguments --";
$extra_arg = 10;
try { var_dump( dir(getcwd(), "stream", $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
