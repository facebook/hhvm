<?hh
/* Prototype: string readlink ( string $path );
   Description: Returns the target of a symbolic link

   Prototype: string realpath ( string $path );
   Description: Returns canonicalized absolute pathname
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing readlink() and realpath() : usage variations ***\n";
$name_prefix = sys_get_temp_dir();
// create temp dir
mkdir("$name_prefix/readlink_realpath_variation2/home/tests/link/", 0777, true);
// create the file
$filename = "$name_prefix/readlink_realpath_variation2/home/tests/link/readlink_realpath_variation2.tmp";
$fp = fopen($filename, "w");
fclose($fp);

echo "\n*** Testing readlink() and realpath() with linkname stored in an array ***\n";
$link_arr = vec[
  "$name_prefix////readlink_realpath_variation2/home/tests/link/readlink_realpath_variation2_link.tmp",
  "$name_prefix/./readlink_realpath_variation2/home/../home//tests//..//..//..//home//readlink_realpath_variation2_link.tmp/"
];

echo "\n-- Testing readlink() and realpath() with softlink, linkname stored inside an array --\n";
// creating the links 
var_dump( symlink($filename, $link_arr[0]) );  
var_dump( readlink($link_arr[0]) ); 
var_dump( realpath($link_arr[0]) ); 
var_dump( symlink($filename, $link_arr[1]) ); 
var_dump( readlink($link_arr[1]) );
var_dump( realpath($link_arr[1]) );

// deleting the link
unlink($link_arr[0]);
unlink($link_arr[1]);  

echo "\n-- Testing readlink() and realpath() with hardlink, linkname stored inside an array --\n";
// creating hard links
var_dump( link($filename, $link_arr[0]) );  
var_dump( readlink($link_arr[0]) );   // invalid because readlink doesn't work with hardlink
var_dump( realpath($link_arr[0]) );
var_dump( link($filename, $link_arr[1]) );  
var_dump( readlink($link_arr[1]) );   // invalid because readlink doesn't work with hardlink
var_dump( realpath($link_arr[1]) );

// delete the links 
unlink($link_arr[0]);
unlink($link_arr[1]);  
  
echo "Done\n";

$name_prefix .= "/readlink_realpath_variation2";
unlink("$name_prefix/home/tests/link/readlink_realpath_variation2.tmp");
rmdir("$name_prefix/home/tests/link/");
rmdir("$name_prefix/home/tests/");
rmdir("$name_prefix/home/");
rmdir("$name_prefix/");
}
