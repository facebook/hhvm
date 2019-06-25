<?hh
/* Prototype  : bool ctype_alpha(mixed $c)
 * Description: Checks for alphabetic character(s) 
 * Source code: ext/ctype/ctype.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_alpha() : basic functionality ***\n";

$orig = setlocale(LC_CTYPE, "C");

$c1 = 'abcXYZ';
$c2 = "Hello, World!";

var_dump(ctype_alpha($c1));
var_dump(ctype_alpha($c2));

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
