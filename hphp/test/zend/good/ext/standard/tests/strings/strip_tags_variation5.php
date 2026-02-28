<?hh
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string
 * Source code: ext/standard/string.c
*/


/*
 * testing functionality of strip_tags() by giving heredoc strings as values for $str argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strip_tags() : usage variations ***\n";

// null here doc string
$null_string = <<<EOT
EOT;

// heredoc string with blank line
$blank_line = <<<EOT

EOT;

// here doc with multiline string
$multiline_string = <<<EOT
<html>hello world</html>
<p>13 &lt; 25</p>
<?hh 1111 &amp; 0000 = 0000 ?>
<b>This is a double quoted string</b>
EOT;

// here doc with different whitespaces
$diff_whitespaces = <<<EOT
<html>hello\r world\t
1111\t\t != 2222\v\v</html>
<? heredoc\ndouble quoted string. with\vdifferent\fwhite\vspaces ?>
EOT;

// here doc with numeric values
$numeric_string = <<<EOT
<html>11 < 12. 123 >22</html>
<p>string</p> 1111\t <b>0000\t = 0000\n</b>
EOT;

// heredoc with quote chars & slash
$quote_char_string = <<<EOT
<html>This's a string with quotes:</html>
"strings in double quote";
'strings in single quote';
<html>this\line is single quoted /with\slashes </html>
EOT;

$res_heredoc_strings = vec[
  //heredoc strings
  $null_string,
  $blank_line,
  $multiline_string,
  $diff_whitespaces,
  $numeric_string,
  $quote_char_string
];

// initialize the second argument
$quotes = "<html><a><?hh";

// loop through $res_heredoc_strings element and check the working on strip_tags()
$count = 1;
for($index =0; $index < count($res_heredoc_strings); $index ++) {
  echo "-- Iteration $count --\n";
  var_dump( strip_tags($res_heredoc_strings[$index], $quotes) );
  $count++;
}

echo "Done\n";
}
