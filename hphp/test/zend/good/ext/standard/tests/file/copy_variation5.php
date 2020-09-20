<?hh
/* Prototype: bool copy ( string $source, string $dest );
   Description: Makes a copy of the file source to dest.
     Returns TRUE on success or FALSE on failure.
*/

/* Test copy() function: Checking case sensitivity in creation of destination file names 
     and the existence and size of destination files
*/
<<__EntryPoint>> function main(): void {
echo "*** Test copy() function: checking case sensitivity in creation of destination file names ***\n";
$src_file_name = __SystemLib\hphp_test_tmppath('copy_variation5.tmp');
$file_handle = fopen($src_file_name, "w");
fwrite( $file_handle, str_repeat(b"Hello2World...\n", 100) );
fclose($file_handle);

/* array of destination file names */
$dest_files = varray[

  /* Checking case sensitiveness */
  "COPY.tmp",
  "COPY.TMP",
  "CopY.TMP"
];

echo "Size of the source file before copy operation => ";
var_dump( filesize($src_file_name) );
clearstatcache();

echo "\n-- Now applying copy() on source file to create copies --";
$count = 1;
foreach($dest_files as $dest_file) {

  echo "\n-- Iteration $count --\n";
  $dest_file_name = __SystemLib\hphp_test_tmppath($dest_file);

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

  $count++;
}

$count = 1;
foreach($dest_files as $dest_file) {
  unlink(__SystemLib\hphp_test_tmppath($dest_file));
  $count++;
}

echo "*** Done ***\n";

unlink(__SystemLib\hphp_test_tmppath('copy_variation5.tmp'));
}
