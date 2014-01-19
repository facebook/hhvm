<?php
/* Prototype: string dirname ( string $path );
   Description: Returns directory name component of path.
*/
$file_paths = array (
  /* simple paths */
  "bar",
  "/foo/bar",
  "foo/bar",
  "/bar",
  "bar/",
  "/bar/",
  "/foo/bar/",
  "foo/bar/",
  "/bar/",
  
  /* path with only files and trailing slashes*/
  "/foo/bar.gz",
  "foo/bar.gz",
  "bar.gz",
  "bar.gz/",
  "/bar.gz",  
  "/bar.gz/",
  "/foo/bar.gz/",
  "foo/bar.gz/",
  "/bar.gz/",  
 
  /* path with file extension and trailing slashes */
  "/.gz",
  ".gz",
  "/foo/.gz",
  ".gz/",
  "/foo/.gz/",
  "foo/.gz/",

  /* paths with binary value to check if the function is binary safe*/
  "foo".chr(0)."bar",
  "/foo".chr(0)."bar/",
  "/foo".chr(0)."bar",
  "foo".chr(0)."bar/",
  "/foo".chr(0)."bar/t.gz"
);

function check_dirname( $paths ) {
   $loop_counter = 0;
   $noOfPaths = count($paths);
   for( ; $loop_counter < $noOfPaths; $loop_counter++ ) {
     echo "\n--Iteration ";
     echo $loop_counter + 1; 
     echo " --\n";
     var_dump( dirname($paths[$loop_counter]) );
   }
}

echo "*** Testing basic operations ***\n";
check_dirname( $file_paths );

echo "Done\n";
?>
