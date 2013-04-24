<?php
/* Prototype:  string tempnam ( string $dir, string $prefix );
   Description: Create file with unique file name.
*/

/* Trying to create the file in a dir with permissions from 0000 to 0777,
     Allowable permissions: files are expected to be created in the input dir 
     Non-allowable permissions: files are expected to be created in '/tmp' dir
*/

echo "*** Testing tempnam() with dir of permissions from 0000 to 0777 ***\n";
$file_path = dirname(__FILE__);
$dir_name = $file_path."/tempnam_variation4";
$prefix = "tempnamVar4.";

mkdir($dir_name);

for($mode = 0000; $mode <= 0777; $mode++) {
  echo "-- dir perms ";
  printf("%o", $mode);
  echo " --\n";
  chmod($dir_name, $mode);
  $file_name = tempnam($dir_name, $prefix);

  if(file_exists($file_name) ) {
    if (realpath(dirname($file_name)) == realpath(sys_get_temp_dir())) {
       $msg = " created in temp dir ";
    }
    else if (dirname($file_name) == $dir_name) {
       $msg = " created in requested dir";
    }
    else {
       $msg = " created in unexpected dir";
    }   
  
    echo $msg."\n";
    unlink($file_name);    
  }
  else {
    print("FAILED: File is not created\n");
  }
}

rmdir($dir_name);

echo "*** Done ***\n";
?>