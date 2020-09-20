<?hh
/* Prototype: bool is_readable ( string $filename );
   Description: Tells whether the filename is readable
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing is_readable(): error conditions ***\n";
try { var_dump( is_readable() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected
try { var_dump( is_readable(1, 2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected

echo "\n*** Testing is_readable() on non-existent file ***\n";
var_dump( is_readable(dirname(__FILE__)."/is_readable.tmp") );

echo "Done\n";
}
