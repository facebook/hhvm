<?hh
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* passing different strings for 'ending' and heredoc string as 'str' to chunk_split()
* 'chunklen' is set to 6E0 for this testcase
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing chunk_split() : different values for 'ending' with heredoc 'str'***\n";

// Initializing required variables
// heredoc string for 'str' argument
$heredoc_str = <<<EOT
This is heredoc string with \t and \n.It also contains
sPeci@! ch@r$ :) & numbers 222.This is \k wrong escape char.
EOT;

$chunklen = 6E+0;

//different values for 'ending'
$values = vec[
  "",  //empty
  " ",  //space
  "a",  //single char
  "ENDING",  //regular string
  "\r\n",  //White space char
  "123",  //Numeric
  ")speci@! ch@r$(",  //String with special chars
];

//loop through each values for 'ending'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1). " --\n";
  try { var_dump( chunk_split($heredoc_str, $chunklen, $values[$count]) ); } catch (Exception $e) { var_dump($e->getMessage()); }
}

echo "Done";
}
