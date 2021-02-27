<?hh

class Foo {
  const ctx C = [rx];
}

function f<reify T>(Foo $a)[$a::C] {
  echo "in f\n";
}

function pure<reify T>($a)[] {
  f<T>($a);
}

<<__EntryPoint>>
function main() {
  $a = new Foo();
  f<int>($a);
  pure<int>($a);
}
