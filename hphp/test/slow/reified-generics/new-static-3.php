<?hh

class C<reify T> {

  static function foo() :mixed{
    return new static();
  }

  function f() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

class B<reify T> extends C<T> {
  static function bar() :mixed{
    return parent::foo();
  }
}

<<__EntryPoint>>
function my_main(): void {
  $x = B::bar();
  $x->f();
}
