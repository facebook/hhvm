<?hh
/* Prototype  : string mb_strtolower(string $sourcestring [, string $encoding])
 * Description: Returns a lowercased version of $sourcestring
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass a Japanese string and a mixed Japanese and ASCII string to mb_strtolower
 * to check correct conversion is occuring (Japanese characters should not be converted).
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_strtolower() : usage variations ***\n";

$string_mixed = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCUEhQLiAwMTIzNO+8le+8lu+8l++8mO+8meOAgg==');
$string_mixed_lower = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCcGhwLiAwMTIzNO+8le+8lu+8l++8mO+8meOAgg==');
$string_all_mb = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CC');

echo "\n-- Mixed string (mulitbyte and ASCII characters) --\n";
$a = mb_strtolower($string_mixed, 'UTF-8');
var_dump(base64_encode($a));
if ($a == $string_mixed_lower) {
    echo "Correctly Converted\n";
} else {
    echo "Incorrectly Converted\n";
}

echo "\n-- Multibyte Only String--\n";
$b = mb_strtolower($string_all_mb, 'UTF-8');
var_dump(base64_encode($b));
if ($b == $string_all_mb) { // Japanese characters only - should not be any conversion
    echo "Correctly Converted\n";
} else {
    echo "Incorrectly Converted\n";
}

echo "Done";
}
