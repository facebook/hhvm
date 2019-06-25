<?hh
/* Prototype  : bool ctype_print(mixed $c)
 * Description: Checks for printable character(s) 
 * Source code: ext/ctype/ctype.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_print() : basic functionality ***\n";

$orig = setlocale(LC_CTYPE, "C");

$c1 = "Hello, World!";
$c2 = null;

var_dump(ctype_print($c1));
var_dump(ctype_print($c2));

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
