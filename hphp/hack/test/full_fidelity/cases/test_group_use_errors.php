<?hh

namespace foo {
    class C { }
    class D { }
}

namespace illegal1 {
  use \foo { C, D };
}

namespace illegal2 {
  use \foo\ { type C, D };
}

namespace illegal3 {
  use \foo\ { type C, type D };
}

namespace legal1 {
  use \foo\ { C, D };
}

namespace legal2 {
  use \foo;
}

namespace illegal4 {
  use \foo as false;
}

namespace legal3 {
  use type \foo\C, \foo\D;
}

namespace legal4 {
  use type \foo\ { C, D };
}
