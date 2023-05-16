<?hh
/* Prototype: bool is_file ( string $filename );
   Description: Tells whether the filename is a regular file
     Returns TRUE if the filename exists and is a regular file
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing is_file(): basic functionality ***\n";

/* Checking with current file */
var_dump( is_file(__FILE__) );

/* Checking with (current) dir */
var_dump( is_file(dirname(__FILE__)) );

$file_name = sys_get_temp_dir().'/'.'is_file_basic.tmp';
/* With non-existing file */
var_dump( is_file($file_name) );
/* With existing file */
fclose( fopen($file_name, "w") );
var_dump( is_file($file_name) );

echo "*** Testing is_file() for its return value type ***\n";
var_dump( is_bool( is_file(__FILE__) ) );
var_dump( is_bool( is_file("/no/such/file") ) );

echo "\n*** Done ***";

unlink($file_name);
}
