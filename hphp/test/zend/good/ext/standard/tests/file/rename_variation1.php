<?hh

/* creating directory */
<<__EntryPoint>> function main(): void {
$file_path = sys_get_temp_dir();
// rename dirs across directories
echo "\n*** Testing rename() : renaming directory across directories ***\n";
$src_dirs = vec[
  /* Testing simple directory tree */
  "$file_path/rename_variation1/",

  /* Testing a dir with trailing slash */
  "$file_path/rename_variation1/",

  /* Testing dir with double trailing slashes */
  "$file_path//rename_variation1//",
];

$dest_dir = "$file_path/rename_variation1_dir";
// create the $dest_dir
mkdir($dest_dir);

$counter = 1;
/* loop through each $src_dirs and rename it to  $dest_dir */
foreach($src_dirs as $src_dir) {
  echo "-- Iteration $counter --\n";

  // create the src dir
  mkdir("$file_path/rename_variation1/");
  // rename the src dir to a new dir in dest dir
  var_dump( rename($src_dir, $dest_dir."/new_dir") );
  // ensure that dir was renamed
  var_dump( file_exists($src_dir) );  // expecting false
  var_dump( file_exists($dest_dir."/new_dir") ); // expecting true

  // remove the new dir
  rmdir($dest_dir."/new_dir");
  $counter++;
}

echo "Done\n";

rmdir($file_path."/rename_variation1_dir");
}
