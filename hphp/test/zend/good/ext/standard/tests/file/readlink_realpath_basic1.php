<?hh
/* Prototype: string readlink ( string $path );
   Description: Returns the target of a symbolic link

   Prototype: string realpath ( string $path );
   Description: Returns canonicalized absolute pathname
*/
/* creating directories, symbolic links and files */
<<__EntryPoint>> function main(): void {
$file_path = sys_get_temp_dir();
mkdir("$file_path/readlink_realpath_basic1/home/test/", 0777, true);

$file_handle1 = fopen("$file_path/readlink_realpath_basic1/home/test/readlink_realpath_basic1.tmp", "w");
$file_handle2 = fopen("$file_path/readlink_realpath_basic1/home/readlink_realpath_basic1.tmp", "w");
$file_handle3 = fopen("$file_path/readlink_realpath_basic1/readlink_realpath_basic1.tmp", "w");
fclose($file_handle1);
fclose($file_handle2);
fclose($file_handle3);

symlink("$file_path/readlink_realpath_basic1/home/test/readlink_realpath_basic1.tmp",
        "$file_path/readlink_realpath_basic1/home/test/readlink_realpath_basic1_link.tmp");
symlink("$file_path/readlink_realpath_basic1/home/readlink_realpath_basic1.tmp",
        "$file_path/readlink_realpath_basic1/home/readlink_realpath_basic1_link.tmp");


echo "*** Testing readlink() and realpath(): with valid and invalid path ***\n";
$linknames = vec[
  /* linknames resulting in valid paths */
  "$file_path/readlink_realpath_basic1/home/readlink_realpath_basic1_link.tmp",
  "$file_path/readlink_realpath_basic1/home/test/readlink_realpath_basic1_link.tmp",
  "$file_path/readlink_realpath_basic1//home/test//../test/./readlink_realpath_basic1_link.tmp",

  /* linknames with invalid linkpath */
  "$file_path///readlink_realpath_basic1/home//..//././test//readlink_realpath_basic1_link.tmp",
  "$file_path/readlink_realpath_basic1/home/../home/../test/../readlink_realpath_basic1_link.tmp",
  "$file_path/readlink_realpath_basic1/..readlink_realpath_basic1_link.tmp",
  "$file_path/readlink_realpath_basic1/home/test/readlink_realpath_basic1_link.tmp/"
];

$counter = 1;
/* loop through $files to read the linkpath of
   the link created from each $file in the above array */
foreach($linknames as $link) {
  echo "\n-- Iteration $counter --\n";
  var_dump( readlink($link) );
  var_dump( realpath($link) );
  $counter++;
}

echo "Done\n";

$name_prefix = $file_path."/readlink_realpath_basic1";
unlink("$name_prefix/home/test/readlink_realpath_basic1.tmp");
unlink("$name_prefix/home/readlink_realpath_basic1.tmp");
unlink("$name_prefix/readlink_realpath_basic1.tmp");
unlink("$name_prefix/home/test/readlink_realpath_basic1_link.tmp");
unlink("$name_prefix/home/readlink_realpath_basic1_link.tmp");
rmdir("$name_prefix/home/test/");
rmdir("$name_prefix/home/");
rmdir("$name_prefix/");
}
