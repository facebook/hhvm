<?hh

// gmp_divexact
<<__EntryPoint>>
function main_most_in_one_x64() :mixed{
$div2 = gmp_divexact("10", "3"); // bogus result
echo gmp_strval($div2) . "\n";
}
