<?hh
/* Prototype  : string strtok ( str $str, str $token )
 * Description: splits a string (str) into smaller strings (tokens), with each token being delimited by any character from token
 * Source code: ext/standard/string.c
*/

/*
 * Testing strtok() : with heredoc strings
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strtok() : with heredoc strings ***\n";

// defining different heredoc strings
$empty_heredoc = <<<EOT
EOT;

$heredoc_with_newline = <<<EOT
\n

EOT;

$heredoc_with_characters = <<<EOT
first line of heredoc string
second line of heredoc string
third line of heredocstring
EOT;

$heredoc_with_newline_and_tabs = <<<EOT
hello\tworld\nhello\nworld\n
EOT;

$heredoc_with_alphanumerics = <<<EOT
hello123world456
1234hello\t1234
EOT;

$heredoc_with_embedded_nulls = <<<EOT
hello\0world\0hello
\0hello\0
EOT;

$heredoc_strings = vec[
                   $empty_heredoc,
                   $heredoc_with_newline,
                   $heredoc_with_characters,
                   $heredoc_with_newline_and_tabs,
                   $heredoc_with_alphanumerics,
                   $heredoc_with_embedded_nulls
                   ];

// loop through each element of the array and check the working of strtok()
// when supplied with different string values

$count = 1;
foreach($heredoc_strings as $string)  {
  echo "\n--- Iteration $count ---\n";
  var_dump( strtok($string, "5o\0\n\t") );
  for($counter = 1; $counter <= 10; $counter++)  {
    var_dump( strtok("5o\0\n\t") );
  }
  $count++;
}


echo "Done\n";
}
