<?hh

class C<reify T> {

  static function foo() {
    return new static();
  }

  function f() {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

class B<reify T> extends C<T> {
  static function bar() {
    return parent::foo();
  }
}

<<__EntryPoint>>
function my_main(): void {
  $x = B::bar();
  $x->f();
}
