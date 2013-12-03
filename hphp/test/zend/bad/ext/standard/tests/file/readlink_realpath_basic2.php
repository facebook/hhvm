<?php
/* Prototype: string readlink ( string $path );
   Description: Returns the target of a symbolic link

   Prototype: string realpath ( string $path );
   Description: Returns canonicalized absolute pathname
*/

/* creating directories, symbolic links and files */
$file_path = dirname(__FILE__);
mkdir("$file_path/readlink_realpath_basic2/home/test/", 0777, true);

$file_handle1 = fopen("$file_path/readlink_realpath_basic2/home/test/readlink_realpath_basic2.tmp", "w");
$file_handle2 = fopen("$file_path/readlink_realpath_basic2/home/readlink_realpath_basic2.tmp", "w");
$file_handle3 = fopen("$file_path/readlink_realpath_basic2/readlink_realpath_basic2.tmp", "w");
fclose($file_handle1);
fclose($file_handle2);
fclose($file_handle3);

echo "\n*** Testing realpath() on filenames ***\n";
$filenames = array (
  /* filenames resulting in valid paths */
  "$file_path/readlink_realpath_basic2/home/readlink_realpath_basic2.tmp",
  "$file_path/readlink_realpath_basic2/readlink_realpath_basic2.tmp",
  "$file_path/readlink_realpath_basic2//home/test//../test/./readlink_realpath_basic2.tmp",
  "$file_path/readlink_realpath_basic2/home//../././readlink_realpath_basic2.tmp",

  /* filenames with invalid path */
  "$file_path///readlink_realpath_basic2/home//..//././test//readlink_realpath_basic2.tmp",
  "$file_path/readlink_realpath_basic2/home/../home/../test/../readlink_realpath_basic2.tmp",
  "$file_path/readlink_realpath_basic2/readlink_realpath_basic2.tmp/"
);

$counter = 1;
/* loop through $files to read the filepath of $file in the above array */
foreach($filenames as $file) {
  echo "\n-- Iteration $counter --\n";
  var_dump( realpath($file) );
  $counter++;
}

echo "Done\n";
?>
<?php
$name_prefix = dirname(__FILE__)."/readlink_realpath_basic2";
unlink("$name_prefix/home/test/readlink_realpath_basic2.tmp");
unlink("$name_prefix/home/readlink_realpath_basic2.tmp");
unlink("$name_prefix/readlink_realpath_basic2.tmp");
rmdir("$name_prefix/home/test/");
rmdir("$name_prefix/home/");
rmdir("$name_prefix/");
?>