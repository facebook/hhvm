<?hh

<<file:__EnableUnstableFeatures("modules")>>

module B;

class Cls {
  static internal function foo_static() {
    echo "in foo_static\n";
  }
  internal function foo() {
    echo "in foo\n";
  }
  <<__SoftInternal>>
  internal function foo_soft() {
    echo "in foo_soft\n";
  }
}

internal function foo() {
  echo "in foo\n";
}

<<__SoftInternal>>
internal function foo_soft() {
  echo "in foo_soft\n";
}

internal class InternalCls {
  function __construct() {
    echo "in InternalCls ctor\n";
  }
  static function foo_static() {
    echo "in foo\n";
  }
  function foo() {
    echo "in foo\n";
  }
}

internal class ReifiedInternalCls {
  function __construct() {
    echo "in ReifiedInternalCls ctor\n";
  }
}

<<__SoftInternal>>
internal class InternalClsSoft {
  function __construct() {
    echo "in InternalClsSoft ctor\n";
  }
}

class Cls2 {
  internal int $x = 1;
  <<__SoftInternal>> internal int $x_soft = 1;
  function __construct(
    internal int $y,
    <<__SoftInternal>> internal int $y_soft
  ) {
    echo "in Cls2 ctor\n";
  }
}
