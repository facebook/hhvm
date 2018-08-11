<?hh

class Test extends DOMDocument {
  <<__Memoize>> public function f() { return 1; }
}
var_dump(new Test()->f());
