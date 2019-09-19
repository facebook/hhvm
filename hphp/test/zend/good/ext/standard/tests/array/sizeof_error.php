<?hh
/* Prototype  : int sizeof(mixed $var)
 * Description: Counts an elements in an array. If Standard PHP Library is installed, 
 * it will return the properties of an object.
 *
 * Alias to functions: count()
 */

// Calling sizeof() with zero and more than expected arguments .
<<__EntryPoint>> function main(): void {
echo "*** Testing sizeof() : error conditions ***\n";

echo "-- Testing sizeof() with zero arguments --\n";
try { var_dump( sizeof() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "-- Testing sizeof() with two arguments --\n";
try { var_dump( sizeof(1, 2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
