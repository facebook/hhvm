<?hh //strict

class Test<T1,T2> {}

type Foo<T<_>> = int;

// cannot use Test<string> as a type of kind * -> *, yet.
function test(Foo<Test<string>> $x) : void {}
