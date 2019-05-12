<?hh

class C {}
class D<reify T> {}
<<__EntryPoint>> function main(): void {
var_dump(HH\ReifiedGenerics\getClassname<C>());
var_dump(HH\ReifiedGenerics\getClassname<D<int>>());
}
