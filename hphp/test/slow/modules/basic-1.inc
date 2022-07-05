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
