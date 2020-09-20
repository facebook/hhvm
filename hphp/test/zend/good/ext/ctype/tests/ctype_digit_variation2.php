<?hh
/* Prototype  : bool ctype_digit(mixed $c)
 * Description: Checks for numeric character(s)
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different integers to ctype_digit() to test which character codes are considered
 * valid decimal digits
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_digit() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

for ($i = 0; $i < 256; $i++) {
    if (ctype_digit($i)) {
        echo "character code $i is a numeric digit\n";
    }
}

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
