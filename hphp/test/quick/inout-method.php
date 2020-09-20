<?hh
class Foo {
  function method(inout $a, inout $b, inout $c): void {
    print("In method: $a, $b, $c\n");
  }
}
<<__EntryPoint>> function main(): void {
$repro = new Foo();
$a = 'hello';
$b = 'world';
$c = 8;
$repro->method(
  inout $a,
  inout $b,
  inout $c,
);
print("After method: $a, $b, $c\n");
}
