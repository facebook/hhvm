<?hh
/* Prototype: bool is_file ( string $filename );
   Description: Tells whether the filename is a regular file
                Returns TRUE if the filename exists and is a regular file
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing is_file() error conditions ***";
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
try { var_dump( is_file() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // Zero No. of args

/* no of args > expected */
$file_handle = fopen($file_path."/is_file_error.tmp", "w");
try { var_dump( is_file( $file_path."/is_file_error.tmp", $file_path."/is_file_error1.tmp") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

/* Non-existing file */
var_dump( is_file($file_path."/is_file_error1.tmp") );

/* Passing resource as an argument */
try { var_dump( is_file($file_handle) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

fclose($file_handle);

echo "\n*** Done ***";
error_reporting(0);
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
if(file_exists($file_path."/is_file_error.tmp")) {
  unlink($file_path."/is_file_error.tmp");
}
if(file_exists($file_path."/is_file_error1.tmp")) {
  unlink($file_path."/is_file_error1.tmp");
}
}
