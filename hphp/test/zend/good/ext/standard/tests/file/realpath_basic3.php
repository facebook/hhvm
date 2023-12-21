<?hh
/* Prototype: string realpath ( string $path );
   Description: Returns canonicalized absolute pathname
*/
<<__EntryPoint>> function main(): void {
echo "\n*** Testing basic functions of realpath() with files ***\n";

/* creating directories and files */
mkdir(sys_get_temp_dir().'/'.'realpath_basic3/home/test', 0777, true);

$file_handle1 = fopen(sys_get_temp_dir().'/'.'realpath_basic3/home/test/realpath_basic3.tmp', "w");
$file_handle2 = fopen(sys_get_temp_dir().'/'.'realpath_basic3/home/realpath_basic3.tmp', "w");
$file_handle3 = fopen(sys_get_temp_dir().'/'.'realpath_basic3/realpath_basic3.tmp', "w");
fclose($file_handle1);
fclose($file_handle2);
fclose($file_handle3);

echo "\n*** Testing realpath() on filenames ***\n";
$filenames = vec[
  /* filenames resulting in valid paths */
  "./realpath_basic3/home/realpath_basic3.tmp",
  "./realpath_basic3/realpath_basic3.tmp",
  "./realpath_basic3//home/test//../test/./realpath_basic3.tmp",
  "./realpath_basic3/home//../././realpath_basic3.tmp",

  /* filenames with invalid path */
  // checking for binary safe
  "./realpath_basic3x000/home/realpath_basic3.tmp",

  ".///realpath_basic3/home//..//././test//realpath_basic3.tmp",
  "./realpath_basic3/home/../home/../test/..realpath_basic3.tmp"
];

chdir(sys_get_temp_dir());

$counter = 1;
/* loop through $files to read the filepath of $file in the above array */
foreach($filenames as $file) {
  echo "\n-- Iteration $counter --\n";
  var_dump( realpath($file) );
  $counter++;
}

echo "Done\n";

$name_prefix = sys_get_temp_dir().'/'.'realpath_basic3';
unlink("$name_prefix/home/test/realpath_basic3.tmp");
unlink("$name_prefix/home/realpath_basic3.tmp");
unlink("$name_prefix/realpath_basic3.tmp");
rmdir("$name_prefix/home/test/");
rmdir("$name_prefix/home/");
rmdir("$name_prefix/");
}
