<?hh

// error: non-existent type constant
class C {
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(C::class, 'T'));
}
