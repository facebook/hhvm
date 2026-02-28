<?hh
/* Prototype: bool copy ( string $source, string $dest );
   Description: Makes a copy of the file source to dest.
     Returns TRUE on success or FALSE on failure.
*/

/* Test copy() function: In creation of destination file names containing special characters
     and checking the existence and size of destination files
*/
<<__EntryPoint>> function main(): void {
echo "*** Test copy() function: destination file names containing special characters ***\n";
$src_file_name = sys_get_temp_dir().'/'.'copy_variation2.tmp';
$file_handle = fopen($src_file_name, "w");
fwrite( $file_handle, str_repeat(b"Hello2World...\n", 100) );
fclose($file_handle);

/* array of destination file names */
$dest_files = vec[

  /* File names containing special(non-alpha numeric) characters */
  "_copy_variation2.tmp", 
  "@copy_variation2.tmp",
  "#copy_variation2.tmp",
  "+copy_variation2.tmp",
  "*copy_variation2.tmp",
  "?copy_variation2.tmp",
  "<copy_variation2.tmp",
  ">copy_variation2.tmp",
  "!copy_variation2.tmp",
  "&copy_variation2.tmp",
  "(copy_variation2.tmp",
  ":copy_variation2.tmp",
  ";copy_variation2.tmp",
  "=copy_variation2.tmp",
  "[copy_variation2.tmp",
  "^copy_variation2.tmp",
  "{copy_variation2.tmp",
  "|copy_variation2.tmp",
  "~copy_variation2.tmp",
  "\$copy_variation2.tmp"
];

echo "Size of the source file before copy operation => ";
var_dump( filesize("$src_file_name") );
clearstatcache();

echo "\n--- Now applying copy() on source file to create copies ---";
$count = 1;
foreach($dest_files as $dest_file) {
  echo "\n-- Iteration $count --\n";
  $dest_file_name = sys_get_temp_dir().'/'.$dest_file;

  echo "Copy operation => ";
  var_dump( copy($src_file_name, $dest_file_name) );

  echo "Existence of destination file => ";
  var_dump( file_exists($dest_file_name) );

  echo "Destination file name => ";
  print($dest_file_name);
  echo "\n";

  echo "Size of source file => ";
  var_dump( filesize($src_file_name) );
  clearstatcache();

  echo "Size of destination file => ";
  var_dump( filesize($dest_file_name) );
  clearstatcache();

  unlink($dest_file_name);

  $count++;
}

echo "*** Done ***\n";
error_reporting(0);
unlink(dirname(__FILE__)."/copy_variation2.tmp");
}
