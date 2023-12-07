<?hh

function f(readonly string $s): void {
  $_ = HH\classname_to_class($s);
}

function g(readonly classname<mixed> $c): void {
  $_ = HH\class_to_classname($c);
}

class C {}

<<__EntryPoint>>
function main(): void {
  f("C");
  g(C::class);
  echo "Done\n";
}
