<?hh



module B;

class C {
  internal function foo() { echo "in ".__METHOD__."\n"; }
  internal static function foo_static() { echo "in ".__METHOD__."\n"; }
}

class D {
  function foo() { echo "in ".__METHOD__."\n"; }
  static function foo_static() { echo "in ".__METHOD__."\n"; }
}

class E {
  internal function foo() { echo "in ".__METHOD__."\n"; }
  internal static function foo_static() { echo "in ".__METHOD__."\n"; }
}
