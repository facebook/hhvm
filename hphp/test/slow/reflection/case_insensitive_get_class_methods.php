<?hh

class A {
  public function b() {}
}

class C extends A {
  public function B() {}
}

interface E {
  public function f();
}

class G implements E {
  public function F() {}
}

interface H {
  public function i();
}
class J {
  public function I() {}
}
class K extends J implements H {}

<<__EntryPoint>>
function main_case_insensitive_get_class_methods() {
  var_dump(get_class_methods('C'));
  var_dump(get_class_methods('G'));
  var_dump(get_class_methods('K'));
}
