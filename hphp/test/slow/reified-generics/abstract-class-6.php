<?hh

abstract class B<T> {}

class C<reify T> extends B<T> {
  function f() :mixed{
    var_dump($this->{'86reified_prop'});
  }
}

<<__EntryPoint>>
function main() :mixed{
    $c = new C<int>();
    $c->f();
}
