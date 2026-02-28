<?hh
/* Prototype: bool is_dir ( string $dirname );
   Description: Tells whether the dirname is a directory
     Returns TRUE if the dirname exists and is a directory, FALSE  otherwise.
*/

/* Testing is_dir() with base and sub dirs */
<<__EntryPoint>> function main(): void {

echo "-- Testing is_dir() with an empty dir --\n";
$dirname = sys_get_temp_dir().'/'.'is_dir_variation1';
mkdir($dirname);
var_dump( is_dir($dirname) );
clearstatcache();

echo "-- Testing is_dir() with a subdir in base dir --\n";
$subdirname = $dirname."/is_dir_variation1_sub";
mkdir($subdirname);
var_dump( is_dir($subdirname) );
var_dump( is_dir($dirname) );

echo "\n*** Done ***";

rmdir($subdirname);
rmdir($dirname);
}
