<?hh

class A {}
class B {}

enum MyEnum : string as string {
  AA = A::class;
  BB = B::class;
}

type MyAlias = MyEnum;

function foo() : MyEnum {
  return MyEnum::AA;
}

function bar(MyAlias $a) : MyAlias {
  return $a;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(foo());
  var_dump(bar(MyEnum::BB));
  MyEnum::assertAll(vec[A::class]);
}
