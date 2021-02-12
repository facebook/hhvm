<?hh

class MyClass {}
enum class MyEnum: MyClass {
  MyClass AAA = new MyClass();
}

class C {
  const MyClass X = MyEnum::AAA;
  private MyClass $prop = MyEnum::AAA;
  private static MyClass $staticProp = MyEnum::AAA;
}
