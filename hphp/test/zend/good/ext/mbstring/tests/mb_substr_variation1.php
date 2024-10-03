<?hh
/* Prototype  : string mb_substr(string $str, int $start [, int $length [, string $encoding]])
 * Description: Returns part of a string
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass different data types as $str to mb_substr() to test behaviour
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_substr() : usage variations ***\n";

// Initialise function arguments not being substituted
$start = 0;
$length = 5;


// heredoc string
$heredoc = <<<EOT
hello world
EOT;
// unexpected values to be passed to $str argument
$inputs = vec[
       // empty data
/*16*/ "",
       '',

       // string data
/*18*/ "string",
       'string',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of mb_substr()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( mb_substr($input, $start, $length)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
