<?hh

abstract class B<T> {}

class C<reify T> extends B<T> {
  function f() {
    var_dump($this->{'86reified_prop'});
  }
}

<<__EntryPoint>>
function main() {
    $c = new C<int>();
    $c->f();
}
