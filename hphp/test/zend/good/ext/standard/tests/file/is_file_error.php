<?hh
/* Prototype: bool is_file ( string $filename );
   Description: Tells whether the filename is a regular file
                Returns TRUE if the filename exists and is a regular file
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing is_file() error conditions ***";

try { var_dump( is_file() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // Zero No. of args

/* no of args > expected */
$file_handle = fopen(sys_get_temp_dir().'/is_file_error.tmp', "w");
try { var_dump( is_file(sys_get_temp_dir().'/is_file_error.tmp', sys_get_temp_dir().'/is_file_error.tmp')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Non-existing file */
var_dump( is_file(sys_get_temp_dir().'/is_file_error1.tmp')) ;

/* Passing resource as an argument */
try { var_dump( is_file($file_handle) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

fclose($file_handle);

echo "\n*** Done ***";

unlink(sys_get_temp_dir().'/is_file_error.tmp');
}
