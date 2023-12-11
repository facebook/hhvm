<?hh
/* Prototype  : string mb_convert_encoding(string $str, string $to_encoding [, mixed $from_encoding])
 * Description: Returns converted string in desired encoding
 * Source code: ext/mbstring/mbstring.c
 */


/*
 * Pass different data types to $to_encoding arg to test behaviour of mb_convert_encoding
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_convert_encoding() : usage variations ***\n";

// Initialise function arguments not being substituted
mb_internal_encoding('utf-8');
$sourcestring = b'hello, world';


// heredoc string
$heredoc = <<<EOT
UTF-8
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $to_encoding argument
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
/*18*/ "UTF-8",
       'UTF-8',
       $heredoc,

       // object data
/*21*/ new classA(),



       // resource variable
/*22*/ $fp
];

// loop through each element of $inputs to check the behaviour of mb_convert_encoding()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump(bin2hex( mb_convert_encoding($sourcestring, $input, 'ISO-8859-1') )); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

fclose($fp);

echo "Done";
}
