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

// unexpected values to be passed to $str argument
$inputs = vec[
       // empty data
/*16*/ "",
       '',

       // string data
/*18*/ b"string",
       b'string',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of mb_strlen()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( mb_strlen($input, $encoding)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
