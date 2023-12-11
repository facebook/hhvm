<?hh
/* Prototype:  string tempnam ( string $dir, string $prefix );
   Description: Create file with unique file name.
*/

/* Creating unique files in various dirs by passing relative paths to $dir arg */
<<__EntryPoint>> function main(): void {
echo "*** Testing tempnam() with absolute and relative paths ***\n";
$dir_name = sys_get_temp_dir().'/'.'tempnam_variation2';
mkdir($dir_name);
$dir_path = $dir_name."/tempnam_variation2_sub";
mkdir($dir_path);

chdir(sys_get_temp_dir());

$dir_paths = vec[
  // absolute paths
  "$dir_path",
  "$dir_path/",
  "$dir_path/..",
  "$dir_path//../",
  "$dir_path/../.././tempnam_variation2",
  "$dir_path/..///tempnam_variation2_sub//..//../tempnam_variation2",
  "$dir_path/BADDIR",
  
  
  // relative paths
  ".",
  "tempname_variation2",
  "tempname_variation2/",
  "tempnam_variation2/tempnam_variation2_sub",
  "tempnam_variation2//tempnam_variation2_sub",  
  "./tempnam_variation2/../tempnam_variation2/tempnam_variation2_sub",
  "BADDIR",  
];

for($i = 0; $i<count($dir_paths); $i++) {
  $j = $i+1;
  echo "\n-- Iteration $j --\n";
  $file_name = tempnam($dir_paths[$i], "tempnam_variation2.tmp");
  
  if( file_exists($file_name) ){

    echo "File name is => ";
    print(realpath($file_name));
    echo "\n";

    echo "File permissions are => ";
    printf("%o", fileperms($file_name) );
    echo "\n";
    
    echo "File created in => ";
    $file_dir = dirname($file_name);
    $dir_req = $dir_paths[$i];
        
    if (realpath($file_dir) == realpath(sys_get_temp_dir())) {
       echo "temp dir\n";
    }
    else if ($file_dir == realpath($dir_req)) {
       echo "directory specified\n";
    }
    else {
       echo "unknown location\n";
    }    
    

  }
  else {
    echo "-- File is not created --";
  }
  
  unlink($file_name);
}

rmdir($dir_path);
rmdir($dir_name);

echo "\n*** Done ***\n";
}
