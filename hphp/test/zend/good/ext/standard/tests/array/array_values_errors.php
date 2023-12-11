<?hh
<<__EntryPoint>> function main(): void {
echo "\n*** Testing error conditions ***\n";
/* Invalid number of args */
try { var_dump( array_values() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // Zero arguments
try { var_dump( array_values(vec[1,2,3], "") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // No. of args > expected
/* Invalid types */
var_dump( array_values("") );  // Empty string
var_dump( array_values(100) );  // Integer
var_dump( array_values(new stdClass) );  // object

echo "Done\n";
}
