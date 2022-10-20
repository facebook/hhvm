<?hh



module A;

function inst($x) {
  $x->foo();
}

function static_method($x) {
  $x::foo_static();
}

<<__EntryPoint>>
function main() {
  include "method-cache-1.inc";
  inst(new C);
  inst(new D);
  inst(new E);
  inst(new C);

  static_method(new C);
  static_method(new D);
  static_method(new E);
  static_method(new C);
}
