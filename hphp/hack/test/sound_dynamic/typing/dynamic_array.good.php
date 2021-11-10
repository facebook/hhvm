<?hh

<<__SupportDynamicType>>
class C1 {}

function test_dyn_array<Tk as arraykey>(
  dynamic $d,
  C1 $c,
  int $i,
  string $s,
  Tk $tk,
  arraykey $ak,
): void {
  $a = $d[$i];
  hh_expect_equivalent<dynamic>($a);
  $a = $d[$s];
  hh_expect_equivalent<dynamic>($a);
  $a = $d[$d];
  hh_expect_equivalent<dynamic>($a);
  $a = $d[$tk];
  hh_expect_equivalent<dynamic>($a);
  $a = $d[$ak];
  hh_expect_equivalent<dynamic>($a);

  $d[$i] = $c;
  $d[$s] = $c;
  $d[$d] = $c;
  $d[$ak] = $c;
  $d[$tk] = $c;
  $d[] = $c;
}
