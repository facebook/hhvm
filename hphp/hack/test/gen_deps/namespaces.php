<?hh

namespace N {
  class Foo {}
  class Bar extends Foo {}
}

namespace {
  class Foo {}
  class Bar extends Foo {}
  class Baz extends Bar {}
  class Qux extends N\Foo {}
}
