<?hh

/* Prototype  : int strlen  ( string $string  )
 * Description: Get string length
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strlen() : basic functionality ***\n";
var_dump(strlen("abcdef"));
var_dump(strlen(" ab de "));
var_dump(strlen(""));
var_dump(strlen("\x90\x91\x00\x93\x94\x90\x91\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"));
echo "===DONE===\n";
}
