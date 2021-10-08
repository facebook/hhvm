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
  hh_show($a);
  $a = $d[$s];
  hh_show($a);
  $a = $d[$d];
  hh_show($a);
  $a = $d[$tk];
  hh_show($a);
  $a = $d[$ak];
  hh_show($a);

  $d[$i] = $c;
  $d[$s] = $c;
  $d[$d] = $c;
  $d[$ak] = $c;
  $d[$tk] = $c;
  $d[] = $c;
}
