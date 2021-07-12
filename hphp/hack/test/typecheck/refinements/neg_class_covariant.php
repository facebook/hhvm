<?hh

function test0<T>(bool $b1, bool $b2, T $t, vec<int> $v) : void {
  $x = null;
  if ($b1) {
    $union = $v;
  } else if ($b2) {
    $union = $t;
  } else {
    $union = 4;
  }
  if ($union is vec<_>) {
    $y = $union;
    $z = $union;
  } else {
    $y = 3;
    $z = $union;
  }
  hh_show($union);
  hh_show($y);
  hh_show($z);
}

function test1<T>(T $m) : void {
  if ($m is vec<_>) {
    $y = $m;
    $z = $m;
  } else {
    $y = 3;
    $z = $m;
  }
  hh_show($m);
  hh_show($y);
  hh_show($z);
}

function test2(mixed $m) : void {
  if ($m is vec<_>) {
    $y = $m;
    $z = $m;
  } else {
    $y = 3;
    $z = $m;
  }
  hh_show($m);
  hh_show($y);
  hh_show($z);
}
