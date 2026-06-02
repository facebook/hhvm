<?hh

function test1(shape('a' => int, ...) $x): void {
  if ($x is shape('b' => int, ...)) {
    hh_show($x);
  } else {
    hh_show($x);
  }
}

function test2(shape('a' => int, ...) $x): void {
  if ($x is shape(?'b' => int, ...)) {
    hh_show($x);
  } else {
    hh_show($x);
  }
}

function test3(shape(?'a' => int, ...) $x): void {
  if ($x is shape('a' => int, ...)) {
    hh_show($x);
  } else {
    hh_show($x);
  }
}
