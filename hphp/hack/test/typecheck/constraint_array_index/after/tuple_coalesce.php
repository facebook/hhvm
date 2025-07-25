<?hh

<<file: __EnableUnstableFeatures('open_tuples')>>

function testTuple(
  (int, string, optional bool, optional float, int...) $t,
): void {
  hh_show($t);
  // These are superfluous: we should lint against them
  $x0 = $t[0] ?? 5;
  $x1 = $t[1] ?? "A";
  // Optional elements
  $x2 = $t[2] ?? false;
  $x3 = $t[2] ?? null;
  $x4 = $t[3] ?? 0.0;
  $x5 = $t[3] ?? 0;
  // Variadic elements
  $x6 = $t[4] ?? null;
  $x7 = $t[5] ?? 3.4;
  hh_expect<int>($x0);
  hh_expect<string>($x1);
  hh_expect<bool>($x2);
  hh_expect<?bool>($x3);
  hh_expect<float>($x4);
  hh_expect<num>($x5);
  hh_expect<?int>($x6);
  hh_expect<num>($x7);
}
