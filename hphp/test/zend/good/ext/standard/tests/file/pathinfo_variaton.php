<?hh
/* Prototype: mixed pathinfo ( string $path [, int $options] );
 * Description: Returns information about a file path
 */
class object_temp {
  public $url_var = "www.foo.com";
  public $html_var = "/var/html/testdir/example.html";
  public $dir_var = "/testdir/foo/test/";
  public $file_var = "/foo//symlink.link";
  public $number = 12345;
}

<<__EntryPoint>>
function main(): void {
  echo "*** Testing pathinfo() with miscelleneous input arguments ***\n";

  $obj = new object_temp();

  $path_arr = vec[
    "www.example.com",
    "/testdir/foo//test/",
    "../foo/test.link",
    "./test/work/scratch/mydir/yourdir/ourdir/test1/test2/test3/test4/test5/test6/test.tmp",
    2.345
  ];

  $paths = vec[
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
  ];

  $counter = 1;
  /* loop through $paths to test each $path in the above array */
  foreach ($paths as $path) {
    echo "-- Iteration $counter --\n";
    try {
      var_dump(pathinfo($path));
    } catch (Exception $e) {
      echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
    }
    try {
      var_dump(pathinfo($path, PATHINFO_DIRNAME));
    } catch (Exception $e) {
      echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
    }
    try {
      var_dump(pathinfo($path, PATHINFO_BASENAME));
    } catch (Exception $e) {
      echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
    }
    try {
      var_dump(pathinfo($path, PATHINFO_EXTENSION));
    } catch (Exception $e) {
      echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
    }
    try {
      var_dump(pathinfo($path, PATHINFO_FILENAME));
    } catch (Exception $e) {
      echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
    }
    $counter++;
  }

  echo "Done\n";
}
