<?hh

class C {}
class D<reify T> {}

var_dump(HH\ReifiedGenerics\getClassname<C>());
var_dump(HH\ReifiedGenerics\getClassname<D<int>>());
