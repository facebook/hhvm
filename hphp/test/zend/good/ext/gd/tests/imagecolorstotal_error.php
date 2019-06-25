<?hh
/* Prototype  : int imagecolorstotal  ( resource $image  )
 * Description: Find out the number of colors in an image's palette
 * Source code: ext/gd/gd.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing imagecolorstotal() : error conditions ***\n";

// Get a resource
$im = fopen(__FILE__, 'r');

echo "\n-- Testing imagecolorstotal() function with Zero arguments --\n";
try { var_dump( imagecolorstotal() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing imagecolorstotal() function with more than expected no. of arguments --\n";
$extra_arg = false;
try { var_dump( imagecolorstotal($im, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing imagecolorstotal() function with a invalid resource\n";
var_dump( imagecolorstotal($im) );

fclose($im); 
echo "===DONE===\n";
}
