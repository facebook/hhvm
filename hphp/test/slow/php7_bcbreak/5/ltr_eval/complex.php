<?hh
class Foo {
  public static int $n = 0;
  public static function incr(): int {
    $x = Foo::$n;
    Foo::$n += 1;
    return $x;
  }
}
<<__EntryPoint>>
function entrypoint_complex(): void {

  $a=dict[];

  $foo = function($i) {
    $n = Foo::$n;
    echo "foo($i): n = $n\n";
    return 0;
  };

  $bar = function($i) {
    $n = Foo::$n;
    echo "bar($i): n = $n\n";
    return 0;
  };

  list(
    $a[Foo::incr() + $foo(0)],
    list(
      $a[Foo::incr() + $foo(10)],
      $a[Foo::incr() + $foo(20)],
    ),
    $a[Foo::incr() + $foo(2)]
  ) = vec[
    "S0: n = " . (Foo::incr() + $bar(0)),
    vec[
      "T0: n = " . (Foo::incr() + $bar(10)),
      "T1: n = " . (Foo::incr() + $bar(20)),
    ],
    "S2: n = " . (Foo::incr() + $bar(2))
  ];

  var_dump($a);
}
