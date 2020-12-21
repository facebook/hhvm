<?hh
class A {
    final function A() { }
}
class B extends A {
    function A() { }
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
