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

echo "-- With default arguments --\n";
//regular string for haystack & needle
var_dump( strrpos("Hello, World", "Hello") );
var_dump( strrpos('Hello, World', "hello") );
var_dump( strrpos("Hello, World", 'World') );
var_dump( strrpos('Hello, World', 'WORLD') );

//single char for needle
var_dump( strrpos("Hello, World", "o") );
var_dump( strrpos("Hello, World", ",") );

//heredoc string for haystack & needle
var_dump( strrpos($heredoc_str, "Hello, World") );
var_dump( strrpos($heredoc_str, 'Hello') );
var_dump( strrpos($heredoc_str, $heredoc_str) );

echo "*** Done ***";
}
