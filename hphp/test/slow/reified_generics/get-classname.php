<?hh

class C {}
class D<reify T> {}
<<__EntryPoint>> function main(): void {
var_dump(HH\ReifiedGenerics\get_classname<C>());
var_dump(HH\ReifiedGenerics\get_classname<D<int>>());
}
