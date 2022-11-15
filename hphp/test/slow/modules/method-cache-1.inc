<?hh



module B;

class C {
  <<__SoftInternal>> internal function foo() { echo "in ".__METHOD__."\n"; }
  <<__SoftInternal>> internal static function foo_static() { echo "in ".__METHOD__."\n"; }
}

class D {
  function foo() { echo "in ".__METHOD__."\n"; }
  static function foo_static() { echo "in ".__METHOD__."\n"; }
}

class E {
  <<__SoftInternal>> internal function foo() { echo "in ".__METHOD__."\n"; }
  <<__SoftInternal>> internal static function foo_static() { echo "in ".__METHOD__."\n"; }
}
