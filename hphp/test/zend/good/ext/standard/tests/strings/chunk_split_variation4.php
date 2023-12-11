<?hh
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* Passing different heredoc strings as 'str' argument to the chunk_split()
* with 'chunklen' 4 and default value of 'ending' that is "\r\n"
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing chunk_split() : heredoc strings as 'str' argument ***\n";

// Initializing required variables
$chunklen = 4;

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
This checks heredoc with $, %, &, chars
EOT4;

// blank heredoc string
$heredoc_blank = <<<EOT5

EOT5;

// heredoc with different white space characters
$heredoc_escchar = <<<EOT6
This checks\t chunk_split()\nEscape\rchars
EOT6;

// heredoc with multiline
$heredoc_multiline= <<<EOT7
This is to check chunk_split
function with multiline
heredoc
EOT7;

// heredoc with quotes and slashes
$heredoc_quote_slash = <<<EOT8
"To check " in heredoc"
I'm sure it'll work also with \
which is single slash
EOT8;

//different heredoc strings for 'str'
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


// loop through each element of the heredoc_arr for 'str'
$count = 0;
foreach($heredoc_arr as $str) {
  echo "-- Iteration ".($count+1). " --\n";
  var_dump( chunk_split( $str, $chunklen) );
  $count++;
};

echo "Done";
}
