<?hh

class C {
}

function foo() :mixed{
  $x = new C;
  if (!$x) {
    echo "Error\n";
  }
}
<<__EntryPoint>> function main(): void {
foo();

echo "End\n";
}
