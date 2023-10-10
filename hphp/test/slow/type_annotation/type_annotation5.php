<?hh

// error: abstract type const
abstract class C {
  abstract const type T;
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(C::class, 'T'));
}
