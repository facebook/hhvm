<?hh
/* Prototype  : bool ctype_cntrl(mixed $c)
 * Description: Checks for control character(s) 
 * Source code: ext/ctype/ctype.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_cntrl() : basic functionality ***\n";

$orig = setlocale(LC_CTYPE, "C");

$c1 = "\r\n\t";
$c2 = "Hello, World!\n";

var_dump(ctype_cntrl($c1));
var_dump(ctype_cntrl($c2));

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
