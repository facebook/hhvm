<?hh
/* Prototype:  string tempnam ( string $dir, string $prefix );
   Description: Create file with unique file name.
*/

/* Creating number of unique files by passing a file name as prefix */
<<__EntryPoint>> function main(): void {
$file_path = sys_get_temp_dir().'/'.'tempnamVar1';
mkdir($file_path);

echo "*** Testing tempnam() in creation of unique files ***\n";
$files = dict[];
for($i=1; $i<=10; $i++) {
  echo "-- Iteration $i --\n";
  $files[$i] = tempnam("$file_path", "tempnam_variation1.tmp");

  if( file_exists($files[$i]) ) {

    echo "File name is => ";
    print($files[$i]);
    echo "\n";

    echo "File permissions are => ";
    printf("%o", fileperms($files[$i]) );
    echo "\n";
    clearstatcache();

    echo "File inode is => ";
    print_r( fileinode($files[$i]) ); //checking inodes
    echo "\n";

    echo "File created in => ";
    $file_dir = dirname($files[$i]);

    if ($file_dir == sys_get_temp_dir()) {
       echo "temp dir\n";
    }
    else if ($file_dir == $file_path) {
       echo "directory specified\n";
    }
    else {
       echo "unknown location\n";
    }
    clearstatcache();
  }
  else {
    print("- File is not created -");
  }
}
for($i=1; $i<=10; $i++) {
  unlink($files[$i]);
}

rmdir($file_path);
echo "*** Done ***\n";
}
