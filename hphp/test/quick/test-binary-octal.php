<?hh
<<__EntryPoint>> function main(): void {
// Successor of maximum signed
$b1 = 0b1000000000000000000000000000000000000000000000000000000000000000;
// Maximum unsigned
$b2 = 0b1111111111111111111111111111111111111111111111111111111111111111;
// Maximum signed
$b3 = 0b0111111111111111111111111111111111111111111111111111111111111111;
// As above but hex
$x1 = 0x8000000000000000;
$x2 = 0xffffffffffffffff;
$x3 = 0x7fffffffffffffff;
// Same as above but octal
$o1 = 01000000000000000000000;
$o2 = 01777777777777777777777;
$o3 = 0777777777777777777777;

// Same as above but decimal (others are outside signed range)
$d3 = 9223372036854775807;

if ($b1!==$x1) echo "b1!==x1\n";
if ($b2!==$x2) echo "b2!==x2\n";
if ($b3!==$x3) echo "b3!==x3\n";
if ($b1!==$o1) echo "b1!==o1\n";
if ($b2!==$o2) echo "b2!==o2\n";
if ($b3!==$o3) echo "b3!==o3\n";
if ($b3!==$d3) echo "b3!==d3\n";
echo "Finished\n";
}
