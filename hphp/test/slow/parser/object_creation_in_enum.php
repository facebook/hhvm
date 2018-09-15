<?hh

class C {}
enum E : string { X = new C(); }

(E::X)->x = 42;
var_dump(E::X);
