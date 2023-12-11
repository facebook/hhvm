<?hh
/* Prototype  : bool ctype_graph(mixed $c)
 * Description: Checks for any printable character(s) except space
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass octal and hexadecimal values to ctype_graph() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_graph() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

$octal_values = vec[061,  062,  063,  064];
$hex_values = varray  [0x31, 0x32, 0x33, 0x34];

echo "\n-- Octal Values --\n";
$iterator = 1;
foreach($octal_values as $c) {
    echo "-- Iteration $iterator --\n";
    var_dump(ctype_graph($c));
    $iterator++;
}

echo "\n-- Hexadecimal Values --\n";
$iterator = 1;
foreach($hex_values as $c) {
    echo "-- Iteration $iterator --\n";
    var_dump(ctype_graph($c));
    $iterator++;
}

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
