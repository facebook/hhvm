<?hh

class MyClass {}
enum class MyEnum: MyClass {
  MyClass AAA = new MyClass();
}

function f(class<MyEnum> $m)[]: void {
  MyEnum::AAA;
  $m::AAA;
}
