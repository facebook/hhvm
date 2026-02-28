<?hh
/* Prototype  :int mb_substr_count(string $haystack, string $needle [, string $encoding])
 * Description: Count the number of substring occurrences
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass different data types as $haystack argument to mb_substr_count() to test behaviour
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "hello, world";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_substr_count() : usage variations ***\n";


// Initialise function arguments not being substituted
$needle = 'world';


// heredoc string
$heredoc = <<<EOT
hello, world
EOT;

// unexpected values to be passed to $haystack argument
$inputs = vec[
       // empty data
/*16*/ "",
       '',

       // string data
/*18*/ "hello, world",
       'hello, world',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of mb_substr_count()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( mb_substr_count($input, $needle) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
