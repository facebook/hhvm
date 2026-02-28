<?hh
/* Prototype  : string chop ( string $str [, string $charlist] )
 * Description: Strip whitespace (or other characters) from the end of a string
 * Source code: ext/standard/string.c
*/

/*
 * Testing chop() : with nulls embedded in input string
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing chop() : string with embedded nulls ***\n";

// defining varous strings with embedded nulls
$strings_with_nulls = vec[
                   "hello\0world",
               "\0hello",
               "hello\0",
               "\0\0hello\tworld\0\0",
               "\\0hello\\0",
               'hello\0\0',
               chr(0),
               chr(0).chr(0),
                    chr(0).'hello'.chr(0),
               'hello'.chr(0).'world'
               ];

$count = 1;
foreach($strings_with_nulls as $string)  {
  echo "\n--- Iteration $count ---\n";
  var_dump( chop($string) );
  var_dump( chop($string, "\0") );
  var_dump( chop($string, '\0') );
  $count++;
}

echo "Done\n";
}
