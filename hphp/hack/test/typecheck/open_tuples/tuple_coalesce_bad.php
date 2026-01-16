<?hh



function testNonVariadicTuple((int, optional string) $t): void {
  $x2 = $t[2] ?? false;
}
function testTuple(
  (int, string, optional bool, optional float, int...) $t,
): void {
  // Optional elements, expect an error here
  $x2 = $t[2];
  hh_expect<bool>($x2); // Despite error
  $x4 = $t[3];
  hh_expect<float>($x4);
  // Variadic elements
  $x6 = $t[4];
  hh_expect<int>($x6);
}
