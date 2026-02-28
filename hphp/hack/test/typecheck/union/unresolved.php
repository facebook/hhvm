<?hh

class A {}

function test(bool $b, ?string $s, nonnull $n, $any): void {
  // make first unresolved
  if ($b) {
    $x = 1;
  } else {
    $x = "";
  }
  hh_show($x);

  // make second unresolved
  if ($b) {
    $y = 1;
  } else if ($b) {
    $y = $s;
  } else {
    $y = true;
  }
  hh_show($y);

  // union them
  if ($b) {
    $z = $x;
  } else {
    $z = $y;
  }
  hh_show($z);

  if ($b) {
    $a = new A();
  } else {
    $a = $x;
  }
  hh_show($a);

  if ($b) {
    $a = new A();
  } else {
    $a = $y;
  }
  hh_show($a);

  if ($b) {
    $a = $x;
  } else {
    $a = $n;
  }
  hh_show($a);

  $a = ($b ? $x : $any);
  hh_show($a);

  // TODO: a test with unresolved of unresolved, which may be doable with
  // interspercing unification and union
}
