<?hh

function union<T1, T2>(T1 $x, T2 $y) : (T1 | T2) {
  return $x;
}

function inter<T1, T2>(T1 $x, T2 $y, (T1 & T2) $z) : (T1 & T2) {
  return $z;
}

function test() : void {
  $u = union<int, num>(4, 5.5);
  hh_show($u);
  $i = inter<string, arraykey>("1", 1, "1");
  hh_show($i);

  $u = union(4, 5.5);
  hh_show($u);
  $i = inter("1", 1, "1");
  hh_show($i);
}
