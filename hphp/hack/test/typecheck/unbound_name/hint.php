<?hh

class Bar {}
type Alias = Bar;

trait T {}
type TraitAlias = T;

// Do not allow typedefs for extends, use traits or record extends
final class Test extends Bar {
  use TraitAlias;
}

// But allow typedefs if they are nested within a concrete class
class Baz<T> {}

final class TestNested extends Baz<Alias> {
}
