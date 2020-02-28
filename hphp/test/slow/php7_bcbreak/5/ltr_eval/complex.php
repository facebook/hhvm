<?hh

class Foo { public static int $n = 0; }

$a=varray[];

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
  $a[Foo::$n++ + $foo(0)],
  list(
    $a[Foo::$n++ + $foo(10)],
    $a[Foo::$n++ + $foo(20)],
  ),
  $a[Foo::$n++ + $foo(2)]
) = varray[
  "S0: n = " . (Foo::$n++ + $bar(0)),
  varray[
    "T0: n = " . (Foo::$n++ + $bar(10)),
    "T1: n = " . (Foo::$n++ + $bar(20)),
  ],
  "S2: n = " . (Foo::$n++ + $bar(2))
];

var_dump($a);
