<?hh
/* Prototype  : number bindec  ( string $binary_string  )
 * Description: Returns the decimal equivalent of the binary number represented by the binary_string  argument.
 * Source code: ext/standard/math.c
 */

/*
 * Pass incorrect input to bindec() to test behaviour
 */

// get a class
class classA
{
}
<<__EntryPoint>> function main(): void {
echo "*** Testing bindec() : error conditions ***\n";

echo "Incorrect number of arguments\n";
try { bindec(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { bindec('01010101111',true); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Incorrect input\n";
bindec(new classA());
}
