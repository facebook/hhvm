<?hh
/* Prototype  : proto array mb_split(string pattern, string string [, int limit])
 * Description: split multibyte string into array by regular expression
 * Source code: ext/mbstring/php_mbregex.c
 * Alias to functions:
 */


// get a class
class classA
{
  public function __toString() :mixed{
    return "UTF-8";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_split() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$pattern = '[a-z]';
$limit = 10;


// heredoc string
$heredoc = <<<EOT
UTF-8
EOT;

// unexpected values to be passed to $encoding argument
$inputs = vec[
       // empty data
/*16*/ "",
       '',

       // string data
/*18*/ "UTF-8",
       'UTF-8',
       $heredoc,
];

// loop through each element of the array for pattern

$iterator = 1;
foreach($inputs as $input) {
      echo "\n-- Iteration $iterator --\n";
      try { var_dump( mb_split($pattern, $input, $limit) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
      $iterator++;
};

echo "Done";
}
