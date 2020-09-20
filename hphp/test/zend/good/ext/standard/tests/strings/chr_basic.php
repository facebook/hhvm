<?hh

/* Prototype  : string chr  ( int $ascii  )
 * Description: Return a specific character
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing chr() : basic functionality ***\n";

echo chr(72). chr(101) . chr(108) . chr(108). chr(111); // Hello
echo chr(10); // "\n"
echo "World";
echo "\n";
echo "===DONE===\n";
}
