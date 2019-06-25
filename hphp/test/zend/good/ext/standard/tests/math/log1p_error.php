<?hh
/* Prototype  : float log1p  ( float $arg  )
 * Description: Returns log(1 + number), computed in a way that is accurate even
 *                when the value of number is close to zero
 * Source code: ext/standard/math.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing log1p() : error conditions ***\n";

echo "\n-- Testing log1p() function with less than expected no. of arguments --\n";
try { log1p(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "\n-- Testing log1p() function with more than expected no. of arguments --\n";
try { log1p(36, true); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===Done===";
}
