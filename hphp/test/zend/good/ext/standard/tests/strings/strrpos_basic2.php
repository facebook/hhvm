<?hh
/* Prototype  : int strrpos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of last occurrence of 'needle' in 'haystack'
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strrpos() function: basic functionality ***\n";
$heredoc_str = <<<EOD
Hello, World
EOD;

echo "-- With all arguments --\n";
//regular string for haystack & needle, with various offsets
var_dump( strrpos("Hello, World", "Hello", 0) );
var_dump( strrpos("Hello, World", 'Hello', 1) );
var_dump( strrpos('Hello, World', 'World', 1) );
var_dump( strrpos('Hello, World', "World", 5) );

//heredoc string for haystack & needle, with various offsets
var_dump( strrpos($heredoc_str, "Hello, World", 0) );
var_dump( strrpos($heredoc_str, 'Hello', 0) );
var_dump( strrpos($heredoc_str, 'Hello', 1) );
var_dump( strrpos($heredoc_str, $heredoc_str, 0) );
var_dump( strrpos($heredoc_str, $heredoc_str, 1) );

//various offsets
var_dump( strrpos("Hello, World", "o", 3) );
var_dump( strrpos("Hello, World", "o", 6) );
var_dump( strrpos("Hello, World", "o", 10) );
echo "*** Done ***";
}
