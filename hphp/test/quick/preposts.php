<?hh
<<__EntryPoint>> function main(): void {
$x = 5;
$x++;
++$x;
$x--;
--$x;

$y1 = $x++ + 2;
$y2 = ++$x + 2;
$y3 = $x-- + 2;
$y4 = --$x + 2;

echo "$y1 $y2 $y3 $y4\n";
}
