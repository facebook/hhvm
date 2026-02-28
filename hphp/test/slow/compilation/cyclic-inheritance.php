<?hh

class A extends C {}
class B extends A {}
class C extends B {}
class D extends C {}

<<__EntryPoint>>
function main() :mixed{
  echo "Shouldn't run\n";
}
