<?hh
/*
 * Prototype  : object dir(string $directory[, resource $context])
 * Description: Directory class with properties, handle and class and methods read, rewind and close
 * Source code: ext/standard/dir.c
 */

/*
 * Passing non resource values to 'context' argument of dir() and see
 * that the function outputs proper warning messages wherever expected.
 */

class classA
{
  public $var;
  public function init() :mixed{
    $this->var = 10;
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing dir() : unexpected values for \$context argument ***\n";

// create the temporary directory

$directory = sys_get_temp_dir().'/'.'dir_variation2';
@mkdir($directory);


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $directory argument
$unexpected_values = varray [
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

       // array data
/*10*/ vec[],
       vec[0],
       vec[1],
       vec[1, 2],
       dict['color' => 'red', 'item' => 'pen'],


       // null data
/*15*/ NULL,
       null,

       // boolean data
/*17*/ true,
       false,
       TRUE,
       FALSE,

       // empty data
/*21*/ "",
       '',

       // string data
/*23*/ "string",
       'string',
       $heredoc,

       // object data
/*26*/ new classA(),


];

// loop through various elements of $unexpected_values to check the behavior of dir()
$iterator = 1;
foreach( $unexpected_values as $unexpected_value ) {
  echo "\n-- Iteration $iterator --";
  try { var_dump( dir($directory, $unexpected_value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
}

echo "Done";

rmdir($directory);
}
