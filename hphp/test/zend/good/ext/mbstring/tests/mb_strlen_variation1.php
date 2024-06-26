<?hh
/* Prototype  : int mb_strlen(string $str [, string $encoding])
 * Description: Get character numbers of a string
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test mb_strlen by passing different data types as $str argument
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return b"Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_strlen() : usage variations ***\n";

// Initialise function arguments not being substituted
$encoding = 'utf-8';


// heredoc string
$heredoc = b<<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $str argument
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

       // string data
/*18*/ b"string",
       b'string',
       $heredoc,

       // object data
/*21*/ new classA(),



       // resource variable
/*22*/ $fp
];

// loop through each element of $inputs to check the behavior of mb_strlen()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( mb_strlen($input, $encoding)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

fclose($fp);

echo "Done";
}
