<?hh

function testCollections(dynamic $x): void {
  $y = vec[];
  $y[] = 5; // $y : vec<int>
  expect_int_vector($y);
  $y[] = $x; // $y : vec<(int | dynamic)>
  hh_show($y);
  $y = Vector { $x }; // $y : Vector<dynamic>
  hh_show($y);
  $y = Map { 5 => $x }; // $y : Map<int, dynamic>
  hh_show($y);
}

function testCollections2(vec<int> $a, dynamic $x): void {
  $a[] = $x; // error, $a is of type vec<int>, incompatible with dynamic
}

function expect_int_vector(vec<int> $_):void {}
