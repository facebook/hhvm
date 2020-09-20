<?hh

record A {
}

record B extends A {
  int x;
}
<<__EntryPoint>> function main(): void {
$b = B['x' => 10];
}
