<?hh

enum Foo : classname<Bar> {
  FOO = 'abc';
  BAR = 'def';
  BAZ = 'hij';
}

type Nus = int;
enum Bar : Nus<XYZ> {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}


function test(): Foo {
  return Foo::FOO;
}

function test2(): Bar {
  return Bar::BAZ;
}
<<__EntryPoint>> function main(): void {
var_dump(test());
var_dump(test2());
}
