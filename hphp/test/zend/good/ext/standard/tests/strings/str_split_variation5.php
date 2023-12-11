<?hh
/* Prototype  : array str_split(string $str [, int $split_length] )
 * Description: Convert a string to an array. If split_length is
                specified, break the string down into chunks each
                split_length characters long.
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* Passing different heredoc strings as 'str' argument to the str_split()
* with 'split_length' 10
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing str_split() : heredoc strings as 'str' argument ***\n";

// Initializing required variables
$split_length = 10;

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
This checks\t str_split()\nEscape\rchars
EOT6;

// heredoc with multiline
$heredoc_multiline= <<<EOT7
This is to check str_split
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
$heredoc_array = vec[
  $heredoc_null,
  $heredoc_blank,
  $heredoc_char,
  $heredoc_str,
  $heredoc_multiline,
  $heredoc_spchar,
  $heredoc_escchar,
  $heredoc_quote_slash
];


// loop through each element of the 'heredoc_array' for 'str'
$count = 0;
foreach($heredoc_array as $str) {
  echo "-- Iteration ".($count+1). " --\n";
  var_dump( str_split($str, $split_length) );
  $count++;
};

echo "Done";
}
