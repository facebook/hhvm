<?hh
/* Prototype:  string tempnam ( string $dir, string $prefix );
   Description: Create file with unique file name.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing tempnam() error conditions ***\n";
$file_path = dirname(__FILE__);

/* More number of arguments than expected */
try { var_dump( tempnam("$file_path", "tempnam_error.tmp", "") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } //Two Valid & One Invalid
try { var_dump( tempnam("$file_path", "tempnam_error.tmp", TRUE) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Less number of arguments than expected */
try { var_dump( tempnam("tempnam_error") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } //One Valid arg
try { var_dump( tempnam("$file_path") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } //One Valid arg
try { var_dump( tempnam("") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } //Empty string
try { var_dump( tempnam(NULL) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } //NULL as arg
try { var_dump( tempnam() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } //Zero args

echo "*** Done ***\n";
}
