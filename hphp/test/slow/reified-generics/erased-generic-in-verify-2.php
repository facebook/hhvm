<?hh

type Tout<T> = T;

class B<T> {}
class C<reify Ta> extends B<int> {}

class D<T> {
  function f(): B<int, Tout<T>> {
    return new B<int, int>();
  }
  function g(B<int, T> $_) :mixed{
    echo "done\n";
  }
}

<<__EntryPoint>>
function main() :mixed{
  $d = new D();
  $d->g($d->f());
}
