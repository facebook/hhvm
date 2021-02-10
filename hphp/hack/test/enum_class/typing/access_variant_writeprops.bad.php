<?hh

class MyClass {}
enum class MyEnum: MyClass {
  MyClass AAA = new MyClass();
}

function f(classname<MyEnum> $m)[]: void {
  MyEnum::AAA;
  $m::AAA;
}
