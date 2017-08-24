<?hh //strict

namespace foo {
    class C { }
    class D { }
}

namespace bar {
  use \foo { C, D };
}

namespace baz {
  use \foo\ { C, D };
}

namespace qux {
  use \foo;
}
