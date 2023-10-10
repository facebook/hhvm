<?hh

namespace Foo;

abstract class Base {
  abstract const type T;
}

interface I {
  const type U = this;
}

class Child extends Base implements I {
  const type T = this;
}

class GrandChild extends Child {
}
<<__EntryPoint>> function main(): void {
\var_dump(type_structure(Child::class, 'T'));
\var_dump(type_structure(GrandChild::class, 'T'));
\var_dump(type_structure(I::class, 'U'));
\var_dump(type_structure(GrandChild::class, 'U'));
}
