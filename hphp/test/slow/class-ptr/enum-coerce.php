<?hh

class C {}

enum E : classname<C> {
  C = nameof C;
}

<<__EntryPoint>>
function main() {
  var_dump(E::coerce(C::class));
}
