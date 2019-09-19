<?hh

<<__ALWAYS_INLINE>>
function foo(inout double $a, inout int $b, inout string $c) {
  $a += 1;
  $b += 2;
  $c += 3;
  $c = (string)$c;
  return 4;
}

<<__EntryPoint>>
function main() {
  list($a, $b, $c) = [2.0, 3, "5"];
  var_dump(
    "one",
    var_dump(1),
    "two",
    foo(inout $a, inout $b, inout $c),
    "three",
    (int)$a,
    $b,
    (int)$c,
  );
}
