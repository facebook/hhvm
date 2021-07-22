<?hh

class A {
  <<__NEVER_INLINE>>
  function foo() { return 1; }
}

function main(A $a) {
  var_dump($a->foo());
}
function handler($name, $obj, inout $args) {
  return shape('value' => "string!");
}


<<__EntryPoint>>
function main_intercept_methods() {
main(new A);
fb_intercept2('A::foo', handler<>);
main(new A);
}
