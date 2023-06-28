<?hh



module A;

function inst($x) :mixed{
  $x->foo();
}

function static_method($x) :mixed{
  $x::foo_static();
}

<<__EntryPoint>>
function main() :mixed{
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
