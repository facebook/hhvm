<?hh

/* Prototype  : array unpack  ( string $format  , string $data  )
 * Description: Unpack data from binary string
 * Source code: ext/standard/pack.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing unpack() : error conditions ***\n";

echo "\n-- Testing unpack() function with no arguments --\n";
try { var_dump( unpack() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing unpack() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump(unpack("I", pack("I", 65534), $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing unpack() function with invalid format character --\n";
$extra_arg = 10;
var_dump(unpack("G", pack("I", 65534)));
echo "===DONE===\n";
}
