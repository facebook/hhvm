<?php
/* 
 * proto string basename(string path [, string suffix])
 * Function is implemented in ext/standard/string.c
 */ 
$file_paths = array (
  /* simple paths */
  array("bar"),
  array("\\foo\\bar"),
  array("foo\\bar"),
  array("\\bar"),

  /* simple paths with trailing slashes */
  array("bar\\"),
  array("\\bar\\"),
  array("\\foo\\bar\\"),
  array("foo\\bar\\"),
  array("\\bar\\"),

  /* paths with suffix removal */
  array("bar.zip", ".zip"),
  array("bar.zip", "bar.zip"),
  array("\\foo\\bar.zip", ".zip"),
  array("foo\\bar.zip", ".zip"),
  array("\\bar.zip", ".zip"),  

  /* paths with suffix and trailing slashes with suffix removal*/
  array("bar.zip\\", ".zip"),
  array("\\bar.zip\\", ".zip"),
  array("\\foo\\bar.zip\\", ".zip"),
  array("foo\\bar.zip\\", ".zip"),
  array("\\bar.zip\\", ".zip"),  
  
  /* paths with basename only suffix, with suffix removal*/
  array("\\.zip", ".zip"),
  array(".zip", ".zip"),
  array("\\foo\\.zip", ".zip"),

  /* paths with basename only suffix & trailing slashes, with suffix removal*/
  array(".zip\\", ".zip"),
  array("\\foo\\.zip\\", ".zip"),
  array("foo\\.zip\\", ".zip"),
);

$file_path_variations = array (
  /* paths with shortcut home dir char, with suffix variation */
  array("C:\\temp\\bar"),
  array("C:\\temp\\bar", ""),
  array("C:\\temp\\bar", NULL),
  array("C:\\temp\\bar", ' '),
  array("C:\\temp\\bar.tar", ".tar"),
  array("C:\\temp\\bar.tar", "~"),
  array("C:\\temp\\bar.tar\\", "~"),
  array("C:\\temp\\bar.tar\\", ""),
  array("C:\\temp\\bar.tar", NULL),
  array("C:\\temp\\bar.tar", ''),
  array("C:\\temp\\bar.tar", " "),

  /* paths with numeric strings */
  array("10.5"),
  array("10.5", ".5"),
  array("10.5", "10.5"),
  array("10"),
  array("105", "5"),
  array("/10.5"),
  array("10.5\\"),
  array("10/10.zip"),
  array("0"),
  array('0'),

  /* paths and suffix given as same */
  array("bar.zip", "bar.zip"),
  array("\\bar.zip", "\\bar.zip"),
  array("\\bar.zip\\", "\\bar.zip\\"),
  array(" ", " "),
  array(' ', ' '),
  array(NULL, NULL),

  /* path with spaces */
  array(" "),
  array(' '),
  
  /* empty paths */
  array(""),
  array(''),
  array(NULL)
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

echo "\n*** Testing possible variations in path and suffix ***\n";
check_basename( $file_path_variations );

echo "\n*** Testing error conditions ***\n";
// zero arguments 
var_dump( basename() );

// more than expected no. of arguments
var_dump( basename("\\blah\\tmp\\bar.zip", ".zip", ".zip") );

// passing invalid type arguments 
$object = new stdclass;
var_dump( basename( array("string\\bar") ) );
var_dump( basename( array("string\\bar"), "bar" ) );
var_dump( basename( "bar", array("string\\bar") ) );
var_dump( basename( $object, "bar" ) );
var_dump( basename( $object ) );
var_dump( basename( $object, $object ) );
var_dump( basename( "bar", $object ) );

echo "Done\n";
?>