<?hh
/* Prototype  : bool chdir(string $directory)
 * Description: Change the current directory
 * Source code: ext/standard/dir.c
 */

/*
 * Pass different data types as $directory argument to test behaviour
 */

// get a class
class classA {
  public $dir_path;

  function __construct($dir) {
    $this->dir_path = $dir;
  }

  public function __toString() :mixed{
    return "$this->dir_path";
  }
}

<<__EntryPoint>>
function main(): void {
  echo "*** Testing chdir() : usage variations ***\n";

  // create the temporary directory

  $dir_path = sys_get_temp_dir().'/'.'chdir_basic';
  @mkdir($dir_path);


  // heredoc string
  $heredoc = <<<EOT
$dir_path
EOT;

  // get a resource variable
  $fp = fopen(__FILE__, "r");

  // unexpected values to be passed to $directory argument
  $inputs = vec[
         // int data
  /*1*/  0,
         1,
         12345,
         -2345,

         // float data
  /*5*/  10.5,
         -10.5,
         12.3456789000e10,
         12.3456789000E-10,
         .5,

         // null data
  /*10*/ NULL,
         null,

         // boolean data
  /*12*/ true,
         false,
         TRUE,
         FALSE,

         // empty data
  /*16*/ "",
         '',
         vec[],

         // string data
  /*19*/ "$dir_path",
         'string',
         $heredoc,

         // object data
  /*22*/ new classA($dir_path),



         // resource variable
  /*23*/ $fp
  ];

  // loop through each element of $inputs to check the behavior of chdir()
  $iterator = 1;
  foreach ($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    try {
      var_dump(chdir($input));
    } catch (Exception $e) {
      echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
    }
    $iterator++;
  }

  fclose($fp);

  echo "===DONE===\n";

  rmdir($dir_path);
}
