<?hh
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* passing different heredoc strings as 'ending' argument to chunk_split()
* 'chunklen' argument is set to 10
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing chunk_split() : different heredoc strings for 'ending' argument ***\n";

// Initializing required variables
$chunklen = 10;
$str = "This is str to check with heredoc ending.This\tcontains,\nspeci@! ch@r$ __with wrong \k escape char 222.";

// Null heredoc string
$heredoc_null = <<<EOT1
EOT1;

// heredoc string with single character
$heredoc_char = <<<EOT2
a
EOT2;

// simple heredoc string
$heredoc_str = <<<EOT3
This is simple heredoc string
EOT3;

// heredoc with special characters
$heredoc_spchar = <<<EOT4
This checks with $, %, &, chars
EOT4;

// blank heredoc string
$heredoc_blank = <<<EOT5

EOT5;

// heredoc with different white space characters
$heredoc_escchar = <<<EOT6
This checks\t and \nwhite space chars
EOT6;

// heredoc with multiline
$heredoc_multiline= <<<EOT7
This is to check chunk_split
function with multiline
heredoc
EOT7;

// heredoc with quotes and slashes
$heredoc_quote_slash = <<<EOT8
"To check " in heredoc".I'm sure it'll \work!
EOT8;

// different heredoc strings for 'ending'
$heredoc_arr = vec[
  $heredoc_null,
  $heredoc_blank,
  $heredoc_char,
  $heredoc_str,
  $heredoc_multiline,
  $heredoc_spchar,
  $heredoc_escchar,
  $heredoc_quote_slash
];


// loop through each element of the heredoc_arr for str
$count = 0;
foreach($heredoc_arr as $value) {
  echo "-- Iteration ".($count+1). " --\n";
  var_dump( chunk_split( $str, $chunklen, $value) );
  $count++;
};

echo "Done";
}
