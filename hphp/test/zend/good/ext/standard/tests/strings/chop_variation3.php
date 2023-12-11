<?hh
/* Prototype  : string chop ( string $str [, string $charlist] )
 * Description: Strip whitespace (or other characters) from the end of a string
 * Source code: ext/standard/string.c
*/

/*
 * Testing chop() : with heredoc strings
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing chop() : with heredoc strings ***\n";

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
$count = 1;
foreach($heredoc_strings as $string)  {
  echo "\n--- Iteration $count ---\n";
  var_dump( chop($string) );
  var_dump( chop($string, "12345o\0\n\t") );
  $count++;
}

echo "Done\n";
}
