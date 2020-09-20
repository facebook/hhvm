<?hh
/* Prototype  : bool ctype_upper(mixed $c)
 * Description: Checks for uppercase character(s) 
 * Source code: ext/ctype/ctype.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_upper() : basic functionality ***\n";

$orig = setlocale(LC_CTYPE, "C");

$c1 = 'HELLOWORLD';
$c2 = "Hello, World!\n";

var_dump(ctype_upper($c1));
var_dump(ctype_upper($c2));

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
