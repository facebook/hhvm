<?hh

function test(Traversable<arraykey> $x): void {
  $s = (new Set($x))->removeAll($x);
  var_dump($s);
  $d = dict($s);
  $d[] = true;
  var_dump($d);
}

<<__EntryPoint>> function main(): void {
  // To hit the specific code path we care about, the input must have > 12
  // values, at least 2 but no more than cap()/8 (so for sizes 13-24 no more
  // than 3) distinct values, and we need to remove all of them via removeAll
  test(vec[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2]);
  test(vec['a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'b', 'c']);
}
