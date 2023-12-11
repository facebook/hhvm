<?hh
/*
 * Prototype  : mixed array_search ( mixed $needle, array $haystack [, bool $strict] )
 * Description: Searches haystack for needle and returns the key if it is found in the array, FALSE otherwise
 * Source Code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions of array_search() ***\n";
/* zero argument */
try { var_dump( array_search() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* unexpected no.of arguments in array_search() */
$var = vec["mon", "tues", "wed", "thurs"];
try { var_dump( array_search(1, $var, 0, "test") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( array_search("test") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* unexpected second argument in array_search() */
$var="test";
var_dump( array_search("test", $var) );
var_dump( array_search(1, 123) );

echo "Done\n";
}
