<?hh

class C {}
enum E : string { X = new C(); }
<<__EntryPoint>> function main(): void {
(E::X)->x = 42;
var_dump(E::X);
}
