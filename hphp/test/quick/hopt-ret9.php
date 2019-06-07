<?hh

class C {
}

function foo() {
  $x = new C;
  if (!$x) {
    echo "Error\n";
  }
}
<<__EntryPoint>> function main() {
foo();

echo "End\n";
}
