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

  // unexpected values to be passed to $directory argument
  $inputs = vec[
         // empty data
  /*1*/  "",
         '',

         // string data
  /*2*/ "$dir_path",
         'string',
         $heredoc,
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

  echo "===DONE===\n";

  rmdir($dir_path);
}
