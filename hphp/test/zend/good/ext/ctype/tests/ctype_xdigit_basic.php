<?hh
/* Prototype  : bool ctype_xdigit(mixed $c)
 * Description: Checks for character(s) representing a hexadecimal digit 
 * Source code: ext/ctype/ctype.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_xdigit() : basic functionality ***\n";
$orig = setlocale(LC_CTYPE, "C"); 

$c1 = 'abcdefABCDEF0123456789';
$c2 = 'face 034';

var_dump(ctype_xdigit($c1));
var_dump(ctype_xdigit($c2));

setlocale(LC_CTYPE, $orig); 
echo "===DONE===\n";
}
