<?hh // strict

class C<reify T> {
  function g() {
    echo "yes\n";
  }
}

function f<reify T>() {
  $x = new T();
  $x->g();
  $y = new T();
  $y->g();
}

<<__EntryPoint>>
function main() {
  f<C<int>>();
}
