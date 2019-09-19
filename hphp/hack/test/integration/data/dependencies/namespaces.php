<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

namespace Test1 {
  class A {}

  function f(int $x) : A {
    return new A();
  }

  namespace NestedTest {
    class A extends UltraNested\B {}

    function g(int $x) : A {
      return new A();
    }

    namespace UltraNested {
        class B {}

        function noop() : void {}
    }

  }
}

namespace Test2 {
  function f(int $x) : \Test1\A {
    return new \Test1\A();
  }
}

namespace Ns {
  function same_name_different_namespaces(int $x) : int {
    \Test1\NestedTest\g($x);
    \Test1\NestedTest\UltraNested\noop();
    if ($x < 0) {
      \Test1\f($x);
      return $x;
    } else {
      \Test2\f($x);
      return -$x;
    }
  }
}
