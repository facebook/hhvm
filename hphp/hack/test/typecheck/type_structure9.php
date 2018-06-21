<?hh // strict

class C {
  const type T = \Foo\D;
}

function test(): void {
  $x = type_structure(\Foo\D::class, 'T');
  hh_show($x['classname']);
}

namespace Foo;

class D {
  const type T = \C;
}

function test(): void {
  $x = type_structure(\C::class, 'T');
  hh_show($x['classname']);
}
