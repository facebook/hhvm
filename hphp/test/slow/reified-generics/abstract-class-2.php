<?hh

abstract class A<reify T> {
  function f(T $x) :mixed{
    echo "done\n";
  }
}

class C extends A<int> {}

<<__EntryPoint>>
function main() :mixed{
  $c = new C();
  $c->f(1);
  $c->f(true);
}
