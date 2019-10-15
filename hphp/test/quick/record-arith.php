<?hh
record Foo {
  x: int,
}
<<__EntryPoint>> function main(): void {
$r = Foo['x'=>2];
echo $r + 1;
}
