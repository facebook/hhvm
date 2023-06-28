<?hh

class C {}

class D {}

class Foo {
  static public function rbar<reify T>(): void {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

function rbaz<reify T>(): void {
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}

function func1($p0 = Foo::rbar<int>) :mixed{
  $p0();
}
function func2($p0 = rbaz<int>) :mixed{
  $p0();
}
function func3($p0 = Foo::rbar<C>) :mixed{
  $p0();
}
function func4($p0 = rbaz<C>) :mixed{
  $p0();
}

<<__EntryPoint>>
function main() :mixed{
  func1();
  func2();
  func3();
  func4();

  // With passed in argument
  func1(Foo::rbar<string>);
  func2(rbaz<string>);
  func3(Foo::rbar<D>);
  func4(rbaz<D>);
}
