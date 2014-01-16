<?php
/* 
 * Prototype  : object dir(string $directory[, resource $context])
 * Description: Directory class with properties, handle and class and methods read, rewind and close
 * Source code: ext/standard/dir.c
 */

/*
 * Passing non resource values to 'context' argument of dir() and see
 * that the function outputs proper warning messages wherever expected.
 */

echo "*** Testing dir() : unexpected values for \$context argument ***\n";

// create the temporary directory
$file_path = dirname(__FILE__);
$directory = $file_path."/dir_variation2";
@mkdir($directory);

// get an unset variable
$unset_var = stream_context_create();
unset($unset_var);

class classA
{
  public $var;
  public function init() {
    $this->var = 10;
  }
}

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $directory argument
$unexpected_values = array (
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
/*10*/ array(),
       array(0),
       array(1),
       array(1, 2),
       array('color' => 'red', 'item' => 'pen'),


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

       // undefined data
/*27*/ @$undefined_var,

       // unset data
/*28*/ @$unset_var
);

// loop through various elements of $unexpected_values to check the behavior of dir()
$iterator = 1;
foreach( $unexpected_values as $unexpected_value ) {
  echo "\n-- Iteration $iterator --";
  var_dump( dir($directory, $unexpected_value) );
  $iterator++;
}

echo "Done";
?>
<?php
$file_path = dirname(__FILE__);
$directory = $file_path."/dir_variation2";

rmdir($directory);
?>