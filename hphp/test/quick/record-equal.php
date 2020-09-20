<?hh
record Foo {
  int x;
  mixed y;
}
<<__EntryPoint>> function main(): void {
$r1 = Foo['x' => 1, 'y' => true];
$r2 = Foo['x' => 1, 'y' => 1];
$r3 = Foo['x' => 2, 'y' => true];
$r4 = Foo['x' => 2, 'y' => true];
var_dump($r1 == $r2);
var_dump($r1 === $r2);
var_dump($r1 == $r3);
var_dump($r3 != $r2);
var_dump($r3 === $r4);
var_dump($r1 === 1);
}
