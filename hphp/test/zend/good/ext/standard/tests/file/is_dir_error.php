<?hh
/* Prototype: bool is_dir ( string $filename );
 *  Description: Tells whether the filename is a regular file
 *               Returns TRUE if the filename exists and is a regular file
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing is_dir() error conditions ***";
try { var_dump( is_dir() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // Zero No. of args


$dir_name = sys_get_temp_dir().'/'.'is_dir_error';
mkdir($dir_name);
try { var_dump( is_dir($dir_name, "is_dir_error1") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // args > expected no.of args

/* Non-existing dir */
var_dump( is_dir("/no/such/dir") );

echo "*** Done ***";

rmdir($dir_name);
}
