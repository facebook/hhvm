<?hh

abstract class A<reify T> {
  function f(T $x) {
    echo "done\n";
  }
}

class C extends A<int> {}

<<__EntryPoint>>
function main() {
  $c = new C();
  $c->f(1);
  $c->f(true);
}
