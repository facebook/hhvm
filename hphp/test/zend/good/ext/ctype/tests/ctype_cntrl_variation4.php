<?hh
/* Prototype  : bool ctype_cntrl(mixed $c)
 * Description: Checks for control character(s)
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass hexadecimal and octal values to ctype_cntrl() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_cntrl() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

$octal_values = vec[01, 02, 03, 04];
$hex_values = vec[0x1, 0x2, 0x3, 0x4];

echo "\n-- Octal Values --\n";
$iterator = 1;
foreach($octal_values as $c) {
    echo "-- Iteration $iterator --\n";
    var_dump(ctype_cntrl($c));
    $iterator++;
}

echo "\n-- Hexadecimal Values --\n";
$iterator = 1;
foreach($hex_values as $c) {
    echo "-- Iteration $iterator --\n";
    var_dump(ctype_cntrl($c));
    $iterator++;
}
setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
