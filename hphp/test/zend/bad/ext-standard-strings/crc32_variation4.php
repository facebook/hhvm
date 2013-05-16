<?php
/* Prototype  : string crc32(string $str)
 * Description: Calculate the crc32 polynomial of a string 
 * Source code: ext/standard/crc32.c
 * Alias to functions: none
*/

/*
 * Testing crc32() : with different heredoc strings passed to the str argument
*/

echo "*** Testing crc32() : with different heredoc strings ***\n";

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

$heredoc_with_hexa_octal = <<<EOT
hello\0\100\xaaworld\0hello
\0hello\0
EOT;

$heredoc_with_long_string = <<<EOT
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbb
cccccccccccccccccccccccccccccccccddddddddddddddddddddddddddddddddd
eeeeeeeeeeeeeeeeeeeeeeeeeeeeffffffffffffffffffffffffffffffffffffff
gggggggggggggggggggggggggggggggggggggggggghhhhhhhhhhhhhhhhhhhhhhhh
111111111111111111111122222222222222222222222222222222222222222222
333333333333333333333333333333333334444444444444444444444444444444
555555555555555555555555555555555555555555556666666666666666666666
777777777777777777777777777777777777777777777777777777777777777777
/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t/t
/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n/n
EOT;

$heredoc_strings = array(
                   $empty_heredoc,
                   $heredoc_with_newline,
                   $heredoc_with_characters,
                   $heredoc_with_newline_and_tabs,
                   $heredoc_with_alphanumerics,
                   $heredoc_with_embedded_nulls,
                   $heredoc_with_hexa_octal,
  		   $heredoc_with_long_string
                   );

// loop to test the function with each heredoc string in the array

$count = 1;
foreach($heredoc_strings as $str) {
  echo "\n-- Iteration $count --\n";
  var_dump( crc32($str) );
  $count++;
}

echo "Done";
?>