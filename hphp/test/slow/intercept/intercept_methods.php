<?hh

class A {
  <<__NEVER_INLINE>>
  function foo() :mixed{ return 1; }
}

function main(A $a) :mixed{
  var_dump($a->foo());
}
function handler($name, $obj, inout $args) :mixed{
  return shape('value' => "string!");
}


<<__EntryPoint>>
function main_intercept_methods() :mixed{
main(new A);
fb_intercept2('A::foo', handler<>);
main(new A);
}
