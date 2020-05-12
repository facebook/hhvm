<?hh

final class Foo {}

type Alias = Foo;

function test<reify T>(): void {
  Foo::class;

  // Allow reified classes
  // TODO: Why is this a typing error but not a naming error?
  T::class;

  // Allow typedefs
  Alias::class;

  // Should error
  NotFound::class;
}

function test2<T>(): void {
  // Do not allow erased generics
  T::class;
}
