<?hh

enum MyEnum : string {
  MY_KEY = 'key';
  ANOTHER_KEY = 'another';
}

class MyClass {
  const MY_CONST = 'key';
  const ANOTHER_CONST = 'another';
}

type MyType1 = array<string, shape(
  MyEnum::MY_KEY => int,
)>;

type MyType2 = shape(
  MyEnum::MY_KEY => int,
);

type MyType3 = shape(
  MyEnum::MY_KEY => shape(
    MyEnum::ANOTHER_KEY => int,
  ),
);

type MyType4 = array<string, shape(
  MyClass::MY_CONST => int,
)>;

type MyType5 = shape(
  MyClass::MY_CONST => int,
);

type MyType6 = shape(
  MyClass::MY_CONST => shape(
    MyClass::ANOTHER_CONST => int,
  ),
);
<<__EntryPoint>> function main(): void {
echo "done.\n";
}
