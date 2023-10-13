<?hh

function test1<TC<TX>>() : void {}

class Foo<TC<TX>> {}

type MyType<TC<TX>> = int;

type MyType2<TX<TY>> = int;

type MyNewType<TC<TX>> = int;
