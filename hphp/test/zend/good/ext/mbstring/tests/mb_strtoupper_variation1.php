<?hh
/* Prototype  : string mb_strtoupper(string $sourcestring [, string $encoding]
 * Description: Returns a uppercased version of $sourcestring
 * Source code: ext/mbstring/mbstring.c
 */

/*
 *
 * Pass different data types as $sourcestring argument to mb_strtoupper to test behaviour
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_strtoupper() : usage variations ***\n";

// Initialise function arguments not being substituted


// heredoc string
$heredoc = <<<EOT
Hello, World
EOT;

// unexpected values to be passed to $sourcestring argument
$inputs = vec[
       // empty data
/*16*/ "",
       '',

       // string data
/*18*/ "String",
       'String',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of mb_strtoupper()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( mb_strtoupper($input) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
