<?hh
/* Prototype  : void closedir([resource $dir_handle])
 * Description: Close directory connection identified by the dir_handle
 * Source code: ext/standard/dir.c
 * Alias to functions: close
 */

/*
 * Pass different data types as $dir_handle argument to closedir() to test behaviour
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing closedir() : usage variations ***\n";


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $dir_handle argument
$inputs = vec[
       // null data
/*10*/ NULL,
       null,
];

// loop through each element of $inputs to check the behavior of closedir()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( closedir($input) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "===DONE===\n";
}
