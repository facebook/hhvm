<?hh

class Foo<T> {}

function test<T1, T2<TX>>() : void where T2<T1> as Foo<T1>  {

}
