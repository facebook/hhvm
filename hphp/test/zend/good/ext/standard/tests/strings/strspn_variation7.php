<?hh
/* Prototype  : proto int strspn(string str, string mask [, int start [, int len]])
 * Description: Finds length of initial segment consisting entirely of characters found in mask.
                If start or/and length is provided works like strspn(substr($s,$start,$len),$good_chars)
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* Testing strspn() : with heredoc string, varying start and default len arguments
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strspn() : with different start values ***\n";

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

// defining array of different heredoc strings
$heredoc_strings = vec[
                   $empty_heredoc,
                   $heredoc_with_newline,
                   $heredoc_with_characters,
                   $heredoc_with_newline_and_tabs,
                   $heredoc_with_alphanumerics,
                   $heredoc_with_embedded_nulls,
                   $heredoc_with_hexa_octal
                   ];

// defining array of different mask strings
$mask_array = vec[
            "",
            '',
            "f\nh\trstie \l",
            'f\n\thrstei \l',
            "\t",
            "t\ e",
            '\t',
            "f\te\h ",
            " \t",
                    "f\t\ih\100e\xa"
                   ];

// defining array of different start values
$start_array = vec[
            0,
            1,
                  2,
            -1,
            -2,
            2147483647,  // max positive integer
            -2147483648,  // min negative integer
                   ];


// loop through each element of the array for heredoc strings, mask strings and start values

$count = 1;

foreach($heredoc_strings as $str)  {
  echo "\n-- Iteration $count --\n";
  foreach($mask_array as $mask)  {
    foreach($start_array as $start)  {
      var_dump( strspn($str,$mask,$start) );  // with default len value
    }
  }
  $count++;
}

echo "Done";
}
