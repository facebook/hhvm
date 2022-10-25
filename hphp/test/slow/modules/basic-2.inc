<?hh



module A;

class Cls {
  static internal function foo_static() {
    echo "in foo_static\n";
  }
  internal function foo() {
    echo "in foo\n";
  }
}
