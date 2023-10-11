<?hh

namespace Foo {
  class ClassA {}
  class ClassB {}
  class ClassC {}
  class ClassD {}
  class ClassE {}
  class ClassF {}
  class ClassG {}
  class ClassH {}
  class ClassI {}
  class ClassJ {}

  function fa(): void {}
  function fb(): void {}
  function fc(): void {}
  function fd(): void {}
  function fe(): void {}
  function ff(): void {}

  const int CONST_A = 1;
  const int CONST_B = 1;
  const int CONST_C = 1;
  const int CONST_D = 1;
  const int CONST_E = 1;
  const int CONST_F = 1;
}

namespace {
  use type Foo\ClassA;
  use type \Foo\ClassB;
  use type Foo\{ClassC, ClassD};
  use type Foo\{
    ClassE,
    ClassF,
  };
  use Foo\{ClassG, ClassH};
  use Foo\{
    ClassI,
    ClassJ,
  };
  use function Foo\fa;
  use function \Foo\fb;
  use function Foo\{fc, fd};
  use function Foo\{
    fe,
    ff,
  };

  use namespace Foo\nsa;
  use namespace \Foo\nsb;
  use namespace Foo\{nsc, nsd};
  use namespace Foo\{
    nse,
    nsf,
  };

  use const Foo\CONST_A;
  use const \Foo\CONST_B;
  use const Foo\{CONST_C, CONST_D};
  use const Foo\{
    CONST_E,
    CONST_F,
  };
}
