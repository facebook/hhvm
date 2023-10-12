<?hh

function test1<T1<_>>() : void {}

class Test1<T1<X>> {}

type foo<T1<X> as SomeClass> = bool;

function test2<T1<TX as Foo<X>>>() : void {}

function test3<T1<TX as Foo<TX>> as Foo<TX>>() : void {}

function test4<T1<+TX>, T2>() : void {}

function test5<C<_>, T2>() : void {}
