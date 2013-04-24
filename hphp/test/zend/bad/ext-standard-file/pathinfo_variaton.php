<?php
/* Prototype: mixed pathinfo ( string $path [, int $options] );
   Description: Returns information about a file path
*/

echo "*** Testing pathinfo() with miscelleneous input arguments ***\n";

$fp = fopen(__FILE__, "r");
unset($fp);

class object_temp {
  public $url_var = "www.foo.com";
  var $html_var = "/var/html/testdir/example.html";
  var $dir_var = "/testdir/foo/test/";
  public $file_var = "/foo//symlink.link";
  var $number = 12345;
}
$obj = new object_temp();

$path_arr = array (
  "www.example.com",
  "/testdir/foo//test/",
  "../foo/test.link",
  "./test/work/scratch/mydir/yourdir/ourdir/test1/test2/test3/test4/test5/test6/test.tmp",
  2.345
);

$paths = array (
  /* pathname containing numeric string */
  0,
  1234,
  -1234,
  2.3456,

  /* pathname as boolean */
  TRUE,
  FALSE,

  /* pathname as an array */
  "./array(1, 2)",
  "array( array(), null)",

  /* pathname as object */
  $obj,

  /* pathname as spaces */
  " ",
  ' ',

  /* empty pathname */
  "",
  '',

  /* pathname as NULL */
  NULL,
  null,
  
  /* filename as resource */
  $fp,

  /* pathname as members of object */
  $obj->url_var,
  $obj->html_var,
  $obj->dir_var,
  $obj->file_var,
  $obj->number,

  /* pathname as member of array */
  $path_arr[0],
  $path_arr[1],
  $path_arr[2],
  $path_arr[3],
  $path_arr[4]
);

$counter = 1;
/* loop through $paths to test each $path in the above array */
foreach($paths as $path) {
  echo "-- Iteration $counter --\n";
  var_dump( pathinfo($path) );
  var_dump( pathinfo($path, PATHINFO_DIRNAME) );
  var_dump( pathinfo($path, PATHINFO_BASENAME) );
  var_dump( pathinfo($path, PATHINFO_EXTENSION) );
  var_dump( pathinfo($path, PATHINFO_FILENAME) );
  $counter++;
}

echo "Done\n";
?>