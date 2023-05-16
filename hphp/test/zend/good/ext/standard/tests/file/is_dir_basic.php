<?hh
/* Prototype: bool is_dir ( string $filename );
   Description: Tells whether the filename is a regular file
     Returns TRUE if the filename exists and is a regular file
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing is_dir(): basic functionality ***\n";
var_dump( is_dir(sys_get_temp_dir()) );
clearstatcache();
var_dump( is_dir(".") );
var_dump( is_dir(__FILE__) );  // expected: bool(false)

$dir_name = sys_get_temp_dir().'/'.'is_dir_basic';
mkdir($dir_name);
var_dump( is_dir($dir_name) );

echo "*** Testing is_dir() for its return value type ***\n";
var_dump( is_bool( is_dir('/') ) );
var_dump( is_bool( is_dir("/no/such/dir") ) );

echo "*** Done ***";

rmdir($dir_name);
}
