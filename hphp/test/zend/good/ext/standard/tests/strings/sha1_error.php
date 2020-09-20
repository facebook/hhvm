<?hh


/* Prototype: string sha1  ( string $str  [, bool $raw_output  ] )
 * Description: Calculate the sha1 hash of a string
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing sha1() : error conditions ***\n";

echo "\n-- Testing sha1() function with no arguments --\n";
try { var_dump( sha1() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing sha1() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( sha1("Hello World",  true, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


echo "===DONE===\n";
}
