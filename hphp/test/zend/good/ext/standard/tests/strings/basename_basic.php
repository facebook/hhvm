<?php
/* Prototype: string basename ( string $path [, string $suffix] );
   Description: Given a string containing a path to a file,
                this function will return the base name of the file. 
                If the filename ends in suffix this will also be cut off.
*/
$file_paths = array (
  /* simple paths */
  array("bar"),
  array("/foo/bar"),
  array("foo/bar"),
  array("/bar"),

  /* simple paths with trailing slashes */
  array("bar/"),
  array("/bar/"),
  array("/foo/bar/"),
  array("foo/bar/"),
  array("/bar/"),

  /* paths with suffix removal */
  array("bar.gz", ".gz"),
  array("bar.gz", "bar.gz"),
  array("/foo/bar.gz", ".gz"),
  array("foo/bar.gz", ".gz"),
  array("/bar.gz", ".gz"),  

  /* paths with suffix and trailing slashes with suffix removal*/
  array("bar.gz/", ".gz"),
  array("/bar.gz/", ".gz"),
  array("/foo/bar.gz/", ".gz"),
  array("foo/bar.gz/", ".gz"),
  array("/bar.gz/", ".gz"),  
  
  /* paths with basename only suffix, with suffix removal*/
  array("/.gz", ".gz"),
  array(".gz", ".gz"),
  array("/foo/.gz", ".gz"),

  /* paths with basename only suffix & trailing slashes, with suffix removal*/
  array(".gz/", ".gz"),
  array("/foo/.gz/", ".gz"),
  array("foo/.gz/", ".gz"),

  /* paths with binary value to check if the function is binary safe*/
  array("foo".chr(0)."bar"),
  array("/foo".chr(0)."bar"),
  array("/foo".chr(0)."bar/"),
  array("foo".chr(0)."bar/"),
  array("foo".chr(0)."bar/test"),
  array("/foo".chr(0)."bar/bar.gz", ".gz"),
  array("/foo".chr(0)."bar/bar.gz")
);

function check_basename( $path_arrays ) {
   $loop_counter = 1;
   foreach ($path_arrays as $path) {
     echo "\n--Iteration $loop_counter--\n"; $loop_counter++;
     if( 1 == count($path) ) { // no suffix provided
       var_dump( basename($path[0]) );
     } else { // path as well as suffix provided,
       var_dump( basename($path[0], $path[1]) );    
     } 
   }
}

echo "*** Testing basic operations ***\n";
check_basename( $file_paths );

echo "Done\n";
?>