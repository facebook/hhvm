<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class A {
  <<__NEVER_INLINE>>
  function foo(named int $a) :mixed{ return $a; }
  function bar(int $a, named int $b = 0) :mixed{ return $a + $b; }
}

function main(A $a) :mixed{
  $a->foo(a=1);
  // This demonstrates that we intercept after named arg defaults are filled out, the
  // intercepted method should print the b named arg name.
  $a->bar(42);
}
function handler($name, $obj, inout $args, ?dict<string, mixed> $namedArgs) :mixed{
  var_dump($args);
  var_dump($namedArgs);
  return shape('value' => "string!");
}

<<__EntryPoint>>
function main_intercept_methods() :mixed{
  main(new A);
  fb_intercept2_named_args('A::foo', handler<>);
  fb_intercept2_named_args('A::bar', handler<>);
  main(new A);
}
