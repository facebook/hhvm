<?hh

<<file:__EnableUnstableFeatures("modules")>>

module B;

internal class InternalCls {
  function __construct() {
    echo "in InternalCls ctor\n";
  }
}

internal class ReifiedInternalCls {
  function __construct() {
    echo "in ReifiedInternalCls ctor\n";
  }
}
