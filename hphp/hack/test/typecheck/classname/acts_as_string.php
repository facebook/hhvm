<?hh // strict

class C {}
trait Tr {}
interface I {}

function foo(): string {
  $x = Tr::class;
  hh_show($x);
  print_stringish(Tr::class);
  print_stringish(C::class);
  print_stringish(I::class);
  print_string(Tr::class);
  print_string(C::class);
  print_string(I::class);
  return C::class;
}

function print_string(string $s): void {
  echo $s, "\n";
}

function print_stringish(Stringish $s): void {
  echo $s, "\n";
}

function string_operations(classname<I> $classname): void {
  $_ = 'prefix'.$classname;
  echo $classname; // is it stringish?
}
