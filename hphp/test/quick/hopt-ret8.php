<?hh

class C {
}

function foo() {
  $x = new C;
}
<<__EntryPoint>> function main(): void {
foo();

echo "End\n";
}
