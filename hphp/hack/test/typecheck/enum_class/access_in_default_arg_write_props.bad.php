<?hh

class MyClass {}
enum class MyClassEnum : MyClass {
  MyClass A = new MyClass();
}

function default_arg(
  MyClass $arg = MyClassEnum::A
)[write_props]: void {}

class EnumClassWrapper {
  public function __construct(
    private MyClass $a = MyClassEnum::A
  )[write_props] {}

  public function top_level(MyClass $a = MyClassEnum::A)[
    write_props
  ] {}
}

class NestedEnumClassAccess {
  public function __construct(
    private EnumClassWrapper $d = new EnumClassWrapper(MyClassEnum::A)
  )[write_props] {}
}
