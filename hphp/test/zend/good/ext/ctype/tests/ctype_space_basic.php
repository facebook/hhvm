<?hh
/* Prototype  : bool ctype_space(mixed $c)
 * Description: Checks for whitespace character(s)
 * Source code: ext/ctype/ctype.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_space() : basic functionality ***\n";

$orig = setlocale(LC_CTYPE, "C");

$c1 = " \t\r\n";
var_dump(ctype_space($c1));

$c2 = "Hello, world!\n";
var_dump(ctype_space($c2));

setlocale(LC_CTYPE, $orig); 
echo "===DONE===\n";
}
