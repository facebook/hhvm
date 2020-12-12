<?hh

class B<reify Ta, Tb> {}

class D<T> {
  function f(): B<int, T> {
    return new B<int, int>();
  }
  function g(B<int, T> $_) {
    echo "done\n";
  }
}

<<__EntryPoint>>
function main() {
  $d = new D();
  $d->g($d->f());
}
