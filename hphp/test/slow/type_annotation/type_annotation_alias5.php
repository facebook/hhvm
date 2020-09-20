<?hh

type MyAlias = int;
type MyAlias1 = MyAlias;
type MyAlias2 = MyAlias1;

class C {
  const type T = MyAlias2::T;
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(C::class, 'T'));
}
