<?hh

class A<T> {}
class B<T> {}

class C<T> {
  function f() {
    return new A<B<T>>();
  }
}
<<__EntryPoint>> function main(): void {
$c = new C<int>();
var_dump($c->f());
}
