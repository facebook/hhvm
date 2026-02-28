<?hh

class C {
}

function foo() :mixed{
  $x = new C;
}
<<__EntryPoint>> function main(): void {
foo();

echo "End\n";
}
