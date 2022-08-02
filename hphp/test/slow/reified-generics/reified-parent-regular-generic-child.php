<?hh

class A<reify T> {
  function f() {
    var_dump($this->{'86reified_prop'});
  }
}

class C<T> extends A<int> {}

<<__EntryPoint>>
function main() {
  $c = new C<string>();
  $c->f();
}
