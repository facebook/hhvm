<?hh
record Foo {
  int x;
}
<<__EntryPoint>> function main(): void {
$r = Foo['x'=>2];
echo $r + 1;
}
