<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class A {
  <<__NEVER_INLINE>>
  function foo(named int $a) :mixed{ return $a; }
}

function foo_intercept(named int $a) :mixed { return $a * 2; }

function foo_handler($name, $obj, inout $args, ?dict<string, mixed> $namedArgs) :mixed{
  $result = \hphp_invoke_callable_named_args(dict[], $args, $namedArgs);
  return shape('value' => $result);
}

function main(A $a) :mixed{
  try {
    $a->foo(a=1);
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}

<<__EntryPoint>>
function main_intercept_methods() :mixed{
  main(new A);
  fb_intercept2_named_args('A::foo', foo_handler<>);
  main(new A);
}
