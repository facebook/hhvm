<?hh

interface ECBase {}

enum class EC : ECBase {}

final abstract class Foo {
  // OK (this is not access to variant that requires impure context)
  const string FOO = EC::class;
}
