<?hh
/* Prototype: void debug_zval_dump ( mixed $variable );
   Description: Dumps a string representation of an internal zend value 
                to output.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";

/* passing zero argument */
try { debug_zval_dump(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
