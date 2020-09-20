<?hh
/* Prototype  : string imap_fetchheader(resource $stream_id, int $msg_no [, int $options])
 * Description: Get the full unfiltered header for a message
 * Source code: ext/imap/php_imap.c
 */

/*
 * Pass different data types as $stream_id argument to test behaviour of imap_fetchheader()
 */

// get a class
class classA
{
  public function __toString() {
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing imap_fetchheader() : usage variations ***\n";

// Initialise function arguments not being substituted
$msg_no = 1;

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get different types of array
$index_array = varray [1, 2, 3];
$assoc_array = darray ['one' => 1, 'two' => 2];

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $stream_id argument
$inputs = varray[

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

       // string data
/*18*/ "string",
       'string',
       $heredoc,

       // array data
/*21*/ varray[],
       $index_array,
       $assoc_array,
       varray['foo', $index_array, $assoc_array],


       // object data
/*25*/ new classA(),

       // undefined data
/*26*/ @$undefined_var,

       // unset data
/*27*/ @$unset_var,
];

// loop through each element of $inputs to check the behavior of imap_fetchheader()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( imap_fetchheader($input, $msg_no) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};
echo "===DONE===\n";
}
