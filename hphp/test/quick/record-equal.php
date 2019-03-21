<?hh
record Foo {
  x: int,
}

$r1 = Foo['x'=>1];
$r2 = Foo['x'=>1];
echo $r1 == $r2;
