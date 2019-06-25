<?hh
/* Prototype: bool is_dir ( string $filename );
   Description: Tells whether the filename is a regular file
     Returns TRUE if the filename exists and is a regular file
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing is_dir(): basic functionality ***\n";
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
var_dump( is_dir($file_path) );
clearstatcache();
var_dump( is_dir(".") );
var_dump( is_dir(__FILE__) );  // expected: bool(false)

$dir_name = $file_path."/is_dir_basic";
mkdir($dir_name);
var_dump( is_dir($dir_name) );

echo "*** Testing is_dir() for its return value type ***\n";
var_dump( is_bool( is_dir($file_path) ) );
var_dump( is_bool( is_dir("/no/such/dir") ) );

echo "*** Done ***";
error_reporting(0);
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
$dir_name = $file_path."/is_dir_basic";
rmdir($dir_name);
}
