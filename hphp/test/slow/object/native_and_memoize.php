<?hh

class Test extends DOMDocument {
  <<__Memoize>> public function f() :mixed{ return 1; }
}

<<__EntryPoint>>
function main_native_and_memoize() :mixed{
var_dump(new Test()->f());
}
