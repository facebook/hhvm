<?hh
namespace {

class C {
  const type T = Foo\D;
  const type U = this;
}

}

namespace Foo {

class D {
  const type T = \C;
  const type U = E;
}

class E {
  const type T = D;
}
<<__EntryPoint>> function main(): void {
\var_dump(type_structure(\C::class, 'T'));
\var_dump(type_structure(\C::class, 'U'));
\var_dump(type_structure(D::class, 'T'));
\var_dump(type_structure(D::class, 'U'));
\var_dump(type_structure(E::class, 'T'));
}
}
