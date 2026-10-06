<?hh

class C {}
trait Tr {}
interface I {}
type Tdef = C;
newtype NTdef = I;

function foo(): string {
  $x = Tr::class;
  hh_show($x);
  print_stringish(nameof Tr);
  print_stringish(nameof C);
  print_stringish(nameof I);
  print_stringish(nameof Tdef);
  print_stringish(nameof NTdef);
  print_string(Tr::class);
  print_string(C::class);
  print_string(I::class);
  print_string(Tdef::class);
  print_string(NTdef::class);
  return C::class;
}

function print_string(string $s): void {
  echo $s, "\n";
}

function print_stringish(Stringish $s): void {
  echo $s, "\n";
}

function string_operations(
  classname<I> $classname,
  typename<Tdef> $tname,
): void {
  $_ = 'prefix'.$classname;
  $_ = 'prefix'.$tname;
  echo $classname; // is it stringish?
  echo $tname;
}
