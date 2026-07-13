<?hh
<<__EntryPoint>> function main(): void {
$x = 5;
$x++;
++$x;
$x--;
--$x;

$t = $x; $x++; $y1 = $t + 2;
++$x; $y2 = $x + 2;
$t = $x; $x--; $y3 = $t + 2;
--$x; $y4 = $x + 2;

echo "$y1 $y2 $y3 $y4\n";
}
