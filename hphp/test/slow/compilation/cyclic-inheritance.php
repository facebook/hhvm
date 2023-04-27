<?hh

class A extends C {}
class B extends A {}
class C extends B {}
class D extends C {}

<<__EntryPoint>>
function main() {
  echo "Shouldn't run\n";
}
