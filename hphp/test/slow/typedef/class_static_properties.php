<?hh

class MyClass {
  const string ASD = "class static properties can be accessed from a type alias to that class\n";

  static public function foobar(): string {
    return "class static methods can be accessed from a type alias to that class\n";
  }
}
type Foo = MyClass;

function test(): void {
  echo Foo::ASD;
  echo Foo::foobar();
}

<<__EntryPoint>>
function main_class_static_properties() :mixed{
  test();
}
