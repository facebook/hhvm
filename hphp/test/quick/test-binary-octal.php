<?hh
<<__EntryPoint>> function main(): void {
// Maximum signed
$b1 = 0b0111111111111111111111111111111111111111111111111111111111111111;
// As above but hex
$x1 = 0x7fffffffffffffff;
// Same as above but octal
$o1 = 0777777777777777777777;

// Same as above but decimal (others are outside signed range)
$d1 = 9223372036854775807;

if ($b1!==$x1) echo "b1!==x1\n";
if ($b1!==$o1) echo "b1!==o1\n";
if ($b1!==$d1) echo "b3!==d3\n";
echo "Finished\n";
}
