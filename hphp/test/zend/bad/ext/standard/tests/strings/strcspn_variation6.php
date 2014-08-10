<?php
/* Prototype  : proto int strcspn(string str, string mask [,int start [,int len]])
 * Description: Finds length of initial segment consisting entirely of characters not found in mask.
                If start or/and length is provided works like strcspn(substr($s,$start,$len),$bad_chars) 
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* Testing strcspn() : with heredoc string, varying mask and default start and len arguments
*/

echo "*** Testing strcspn() : with different mask strings ***\n";

// initialing required variables
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

$heredoc_strings = array(
                   $empty_heredoc,
                   $heredoc_with_newline,
                   $heredoc_with_characters,
                   $heredoc_with_newline_and_tabs,
                   $heredoc_with_alphanumerics,
                   $heredoc_with_embedded_nulls,
                   $heredoc_with_hexa_octal
                   );

// defining array of mask strings
$mask_array = array(
		    "",
		    '',
		    "\n\trsti \l",
		    '\n\trsti \l',
		    "\t",
		    "t\ ",
		    '\t',
		    "\t\ ",
		    " \t",
                    "\t\i\100\xaa"
                   );
		

// loop through each element of the arrays for string and mask arguments

$count = 1;
foreach($heredoc_strings as $str) {
  echo "\n-- Iteration $count --\n";
  foreach($mask_array as $mask) {
      var_dump( strcspn($str,$mask) ); // with default start and len value
  }
  $count++;
}

echo "Done"
?>
