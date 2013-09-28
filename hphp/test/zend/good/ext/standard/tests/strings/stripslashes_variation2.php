<?php
/* Prototype  : string stripslashes ( string $str )
 * Description: Returns an un-quoted string
 * Source code: ext/standard/string.c
*/

/*
 * Test stripslashes() with various strings containing characters thats can be backslashed.
 * First adding slashes using addslashes() and then removing the slashes using stripslashes() 
*/

echo "*** Testing stripslashes() : with various strings containing backslashed characters ***\n";

// initialising a heredoc string
$heredoc_string = <<<EOT
This is line 1 of 'heredoc' string
This is line 2 of "heredoc" string
EOT;

$heredoc_null_string =<<<EOT
EOT;
$heredoc_string_only_backslash =<<<EOT
\
EOT;
$heredoc_string_only_single_quote =<<<EOT
'
EOT;
$heredoc_string_only_double_quote =<<<EOT
"
EOT;
 
// initialising the string array

$str_array = array( 
                    // string without any characters that can be backslashed
                    'Hello world',
 
                    // string with single quotes
                    "how're you doing?", 
                    "don't disturb u'r neighbours",
                    "don't disturb u'r neighbours''",
                    '',
                    '\'',
                    "'",
                    $heredoc_string_only_single_quote,
                    
                    // string with double quotes
                    'he said, "he will be on leave"',
                    'he said, ""he will be on leave"',
                    '"""PHP"""',
                    "",
                    "\"",
                    '"',
 		    "hello\"",
                    $heredoc_string_only_double_quote,
                         
                    // string with backslash characters
                    'Is your name Ram\Krishna?',
                    '\\0.0.0.0',
                    'c:\php\testcase\stripslashes',
                    '\\',
                    $heredoc_string_only_backslash,

                    // string with nul characters
                    'hello'.chr(0).'world',
                    chr(0).'hello'.chr(0),
                    chr(0).chr(0).'hello',
                    chr(0),

                    // mixed strings
                    "'\\0.0.0.0'",
                    "'\\0.0.0.0'".chr(0),
                    chr(0)."'c:\php\'",
                    '"\\0.0.0.0"',
                    '"c:\php\"'.chr(0)."'",
                    '"hello"'."'world'".chr(0).'//',

		    // string with hexadecimal number
                    "0xABCDEF0123456789",
                    "\x00",
                    '!@#$%&*@$%#&/;:,<>',
                    "hello\x00world",

                    // heredoc strings
                    $heredoc_string,
                    $heredoc_null_string
                  );

$count = 1;
// looping to test for all strings in $str_array
foreach( $str_array as $str )  {
  echo "\n-- Iteration $count --\n";
  $str_addslashes = addslashes($str);
  var_dump("The string after addslashes is:", $str_addslashes);
  $str_stripslashes = stripslashes($str_addslashes);
  var_dump("The string after stripslashes is:", $str_stripslashes);
  if( strcmp($str, $str_stripslashes) != 0 )
    echo "\nError: Original string and string from stripslash() donot match\n";
  $count ++;
}

echo "Done\n";
?>