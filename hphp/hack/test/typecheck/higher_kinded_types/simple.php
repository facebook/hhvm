<?hh // strict

class Test<T1<T2>,T3<T4, T5<T6>>> {}

type TypeTest<T1<T2>,T3<T4, T5<T6>>>  = int;

newtype NewTypeTest<T1<T2>,T3<T4, T5<T6>>>  = int;

function test<T1<T2>,T3<T4, T5<T6>>> () : void {}
