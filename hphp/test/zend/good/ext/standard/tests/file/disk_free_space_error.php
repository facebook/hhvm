<?hh
/*
 *  Prototype: float disk_free_space( string directory )
 *  Description: Given a string containing a directory, this function will
 *               return the number of bytes available on the corresponding
 *               filesystem or disk partition
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";

try { var_dump( disk_free_space() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // Zero Arguments
try { var_dump( diskfreespace() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

try { var_dump( disk_free_space( "junk", "extra argument") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // More than valid number of arguments
try { var_dump( diskfreespace( "junk", "extra argument") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


var_dump( disk_free_space( __SystemLib\hphp_test_tmppath('dir1') )); // Invalid directory
var_dump( diskfreespace( __SystemLib\hphp_test_tmppath('dir1') ));

$fh = fopen( __SystemLib\hphp_test_tmppath('disk_free_space.tmp'), "w" );
fwrite( $fh, (string)" Garbage data for the temporary file" );
var_dump( disk_free_space( __SystemLib\hphp_test_tmppath('disk_free_space.tmp') )); // file input instead of directory
var_dump( diskfreespace( __SystemLib\hphp_test_tmppath('disk_free_space.tmp') ));
fclose($fh);

echo"\n-- Done --";

unlink(__SystemLib\hphp_test_tmppath('disk_free_space.tmp'));
}
