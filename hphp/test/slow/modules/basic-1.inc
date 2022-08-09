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
}

internal function foo() {
  echo "in foo\n";
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
