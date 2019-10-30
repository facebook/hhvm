<?hh
/*
 *  Prototype: float disk_free_space( string directory )
 *  Description: Given a string containing a directory, this function will
 *               return the number of bytes available on the corresponding
 *               filesystem or disk partition
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
try { var_dump( disk_free_space() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // Zero Arguments
try { var_dump( diskfreespace() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

try { var_dump( disk_free_space( $file_path, "extra argument") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // More than valid number of arguments
try { var_dump( diskfreespace( $file_path, "extra argument") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


var_dump( disk_free_space( $file_path."/dir1" )); // Invalid directory
var_dump( diskfreespace( $file_path."/dir1" ));

$fh = fopen( $file_path."/disk_free_space.tmp", "w" );
fwrite( $fh, (string)" Garbage data for the temporary file" );
var_dump( disk_free_space( $file_path."/disk_free_space.tmp" )); // file input instead of directory
var_dump( diskfreespace( $file_path."/disk_free_space.tmp" ));
fclose($fh);

echo"\n-- Done --";
error_reporting(0);
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
unlink($file_path."/disk_free_space.tmp");
}
