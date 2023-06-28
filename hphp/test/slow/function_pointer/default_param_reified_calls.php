<?hh

class C {}

class Foo {
  static public function rbar<reify T>(): void {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

function rbaz<reify T>(): void {
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}

function func1($p0 = Foo::rbar<int>()) :mixed{
}
function func2($p0 = rbaz<int>()) :mixed{
}
function func3($p0 = Foo::rbar<C>()) :mixed{
}
function func4($p0 = rbaz<C>()) :mixed{
}

<<__EntryPoint>>
function main() :mixed{
  func1();
  func2();
  func3();
  func4();
}
