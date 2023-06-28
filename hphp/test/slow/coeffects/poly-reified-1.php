<?hh

class Foo {
  const ctx C = [rx];
}

function f<reify T>(Foo $a)[$a::C] :mixed{
  echo "in f\n";
}

function pure<reify T>($a)[] :mixed{
  f<T>($a);
}

<<__EntryPoint>>
function main() :mixed{
  $a = new Foo();
  f<int>($a);
  pure<int>($a);
}
