<?php
/* Prototype: string dirname ( string $path );
   Description: Returns directory name component of path.
*/
class temp
{ 
   function __toString() {
     return "Object";
   }
}

$file_path_variations = array (
  /* home dir shortcut char */
  "~/home/user/bar",
  "~/home/user/bar/",
  "~/home/user/bar.tar",
  "~/home/user/bar.tar/",

  /* with hotname:dir notation */
  "hostname:/home/user/bar.tar",
  "hostname:/home/user/tbar.gz/",
  "hostname:/home/user/My Pics.gz",
  "hostname:/home/user/My Pics.gz/",
  "hostname:/home/user/My Pics/",
  "hostname:/home/user/My Pics",
  
  /* path containing numeric string */
  "10.5",
  "/10.5",
  "/10.5/",
  "10.5/",
  "10/10.gz",
  '0',
  "0",
  
  /* object */
  new temp,

  /* path as spaces */
  " ",
  ' ',

  /* empty path */
  "",
  '',
  NULL,
  null
);

function check_dirname( $paths ) {
   $loop_counter = 0;
   $noOfPaths = count($paths);
   for( ; $loop_counter < $noOfPaths; $loop_counter++ ) {
     echo "\n--Iteration ";
     echo $loop_counter +1;
     echo " --\n"; 
     var_dump( dirname($paths[$loop_counter]) );
   }
}

echo "*** Testing possible variations in path ***\n";
check_dirname( $file_path_variations );

echo "Done\n";
?>