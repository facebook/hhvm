<?hh
/*
 *  Prototype: float disk_total_space( string $directory );
 *  Description: given a string containing a directory, this function
 *               will return the total number of bytes on the corresponding
 *               filesystem or disk partition
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";

try { var_dump( disk_total_space() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // Zero Arguments

try { var_dump( disk_total_space( "junk", "extra argument") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // More than valid number of arguments


var_dump( disk_total_space( sys_get_temp_dir().'/'.'dir1') ); // Invalid directory

$fh = fopen( sys_get_temp_dir().'/'.'disk_total_space.tmp', "w") ;
fwrite( $fh, (string)" Garbage data for the temporary file" );
var_dump( disk_total_space( sys_get_temp_dir().'/'.'disk_total_space.tmp') ); // file input instead of directory
fclose($fh);

echo"\n--- Done ---";

unlink(sys_get_temp_dir().'/'.'disk_total_space.tmp');
}
