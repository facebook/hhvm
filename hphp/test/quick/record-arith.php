<?hh
record Foo {
  x: int,
}

$r = Foo['x'=>2];
echo $r + 1;
