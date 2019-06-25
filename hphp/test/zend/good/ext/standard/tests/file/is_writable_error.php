<?hh
/* Prototype: bool is_writable ( string $filename );
   Description: Tells whether the filename is writable.

   is_writeable() is an alias of is_writable()
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing is_writable(): error conditions ***\n";
try { var_dump( is_writable() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args < expected
try { var_dump( is_writeable() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n*** Testing is_writeable(): error conditions ***\n";
try { var_dump( is_writable(1, 2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected
try { var_dump( is_writeable(1, 2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n*** Testing is_writable() on non-existent file ***\n";
var_dump( is_writable(dirname(__FILE__)."/is_writable") );
var_dump( is_writeable(dirname(__FILE__)."/is_writable") );

echo "Done\n";
}
