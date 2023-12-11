<?hh

<<__ALWAYS_INLINE>>
function foo(inout float $a, inout int $b, inout string $c) :mixed{
  $a += 1;
  $b += 2;
  $c = HH\Lib\Legacy_FIXME\cast_for_arithmetic($c);
  $c += 3;
  $c = (string)$c;
  return 4;
}

<<__EntryPoint>>
function main() :mixed{
  list($a, $b, $c) = vec[2.0, 3, "5"];
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
