<?hh

namespace SomeNS {

enum Foo: string {
  VALUE = __CLASS__;
}

enum Bar: string {
  VALUE = __CLASS__;
}

enum Baz: string {
  VALUE = __CLASS__;
}

}

namespace MyNS {


namespace MySubNS {

enum MyEnum: string {
  VALUE = 'Herp';
}

}

use type SomeNS\Foo;
use SomeNS\Bar;

enum MyEnum: string {
  FOO = 'Herp';
}

type MyShape = shape(
  MyEnum::FOO => string,
);

type T = MyEnum;

type UsesFoo = shape(Foo::VALUE => int);
type UsesBar = shape(Bar::VALUE => int);
type UsesBaz = shape(\SomeNS\Baz::VALUE => int);

type ExplicitRelative = shape(namespace\MyEnum::FOO => int);
type ImplicitRelative = shape(MySubNS\MyEnum::VALUE => int);
<<__EntryPoint>> function main(): void {
\var_dump(type_structure(MyShape::class));
\var_dump(type_structure(T::class));
\var_dump(type_structure(UsesFoo::class));
\var_dump(type_structure(UsesBar::class));
\var_dump(type_structure(UsesBaz::class));
\var_dump(type_structure(ExplicitRelative::class));
\var_dump(type_structure(ImplicitRelative::class));

}
}
