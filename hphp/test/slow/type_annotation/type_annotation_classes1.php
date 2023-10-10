<?hh

class MyClass {
}

interface MyInterface {
}

trait MyTrait {
}

enum MyEnum : int {
  VAL1 = 1;
  VAL2 = 2;
}

class C {
  const type T1 = MyClass;
  const type T2 = MyInterface;
  const type T3 = MyTrait;
  const type T4 = MyEnum;
  const type T5 = MyClass<MyEnum>;
  const type T6 = MyClass<this>;
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(C::class, 'T1'));
var_dump(type_structure(C::class, 'T2'));
var_dump(type_structure(C::class, 'T3'));
var_dump(type_structure(C::class, 'T4'));
var_dump(type_structure(C::class, 'T5'));
var_dump(type_structure(C::class, 'T6'));
}
