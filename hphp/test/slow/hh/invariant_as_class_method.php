<?hh

class A {
  public static function invariant() {
    var_dump("works");
  }
}

<<__EntryPoint>>
function main_invariant_as_class_method() {
A::invariant();
}
