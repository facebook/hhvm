<?hh

<<__ALWAYS_INLINE>>
function foo(
  inout float $a,
  inout int $b,
  inout string $c,
  inout $x,
  inout $y,
  inout $z,
  inout $q,
  inout $r,
  inout $s,
) :mixed{
  $a += 1;
  $b += 2;
  $c = HH\Lib\Legacy_FIXME\cast_for_arithmetic($c);
  $c += 3;
  $c = (string)$c;

  $x = __hhvm_intrinsics\launder_value($x);
  $y = __hhvm_intrinsics\launder_value($y);
  $z = __hhvm_intrinsics\launder_value($z);
  $q = __hhvm_intrinsics\launder_value($q);
  $r = __hhvm_intrinsics\launder_value($r);
  $s = __hhvm_intrinsics\launder_value($s);

  return 4;
}

<<__EntryPoint>>
function main() :mixed{
  // HHBBC should optimize these away...
  $x = 1;
  $y = 2;
  $z = 3;
  $q = 4;
  $r = 5;
  $s = 6;

  list($a, $b, $c) = vec[2.0, 3, "5"];
  var_dump(
    "one",
    var_dump(1),
    "two",
    foo(
      inout $a,
      inout $b,
      inout $c,
      inout $x,
      inout $y,
      inout $z,
      inout $q,
      inout $r,
      inout $s,
    ),
    "three",
    (int)$a,
    $b,
    (int)$c,
  );
}
