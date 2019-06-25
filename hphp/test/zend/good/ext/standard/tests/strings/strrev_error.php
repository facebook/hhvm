<?hh
/* Prototype  : string strrev(string $str);
 * Description: Reverse a string 
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strrev() : error conditions ***\n";
echo "-- Testing strrev() function with Zero arguments --";
try { var_dump( strrev() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strrev() function with more than expected no. of arguments --";
try { var_dump( strrev("string", 'extra_arg') ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "*** Done ***";
}
