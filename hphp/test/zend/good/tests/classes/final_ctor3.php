<?hh
class A {
    final function A() :mixed{ }
}
class B extends A {
    function A() :mixed{ }
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
