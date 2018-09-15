<?hh

class Test extends DOMDocument {
  <<__Memoize>> public function f() { return 1; }
}

<<__EntryPoint>>
function main_native_and_memoize() {
var_dump(new Test()->f());
}
