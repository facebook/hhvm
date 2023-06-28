<?hh
function gcd($x, $y) :mixed{
  if ($x >= $y) {
    $a = $x;
    $b = $y;
  } else {
    $a = $y;
    $b = $x;
  }
  if ($b <= 0) {
    if ($b >= 0) {
      return $a+0;
    }
  }
  return gcd($b+0, $a-$b);
}
<<__EntryPoint>> function main(): void {
echo gcd(330, 462);
echo "\n";
}
