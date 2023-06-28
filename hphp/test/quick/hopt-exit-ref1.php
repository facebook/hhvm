<?hh

class C {
}

function foo() :mixed{
  $x = new C;
  if ($x) {
    echo "not null\n";
  } else {
    echo "null\n";
  }
  $x = 1;
  if ($x) return 1;
}

<<__EntryPoint>> function main(): void {
foo();
}
