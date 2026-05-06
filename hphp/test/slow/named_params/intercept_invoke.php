<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class A {
  <<__NEVER_INLINE>>
  function foo(named int $a) :mixed{ return $a; }

  <<__NEVER_INLINE>>
  function bar(int $x, named int $a) :mixed{ return 10 * $x + $a; }

  <<__NEVER_INLINE>>
  function baz(named int $c) :mixed { return $c; }

  <<__NEVER_INLINE>>
  function multi(named vec<int> $a, named int $b) :mixed { return 100 * $a[0] + $b; }

  public static function qux(named int $d) { return $d; }
}

function foo_intercept(named int $a) :mixed { return $a * 2; }

function bar_intercept(int $x, named int $a) :mixed { return 100 * $x + $a; }

function multi_intercept(named vec<int> $a, named int $b) :mixed { return 100 * $a[0] + $b; }

class B {
  public static function baz_intercept(named int $c) :mixed { return 42 * $c; }
}

function foo_handler($name, $obj, inout $args, ?dict<string, mixed> $named): mixed{
  $result = \hphp_invoke_callable_named_args(foo_intercept<>, $args, $named);
  return shape('value' => $result);
}

function bar_handler($name, $obj, inout $args, ?dict<string, mixed> $named): mixed{
  $result = \hphp_invoke_callable_named_args(bar_intercept<>, $args, $named);
  return shape('value' => $result);
}

function baz_handler($name, $obj, inout $args, ?dict<string, mixed> $named): mixed{
  $result = \hphp_invoke_callable_named_args(B::baz_intercept<>, $args, $named);
  return shape('value' => $result);
}

function qux_handler($name, $obj, inout $args, ?dict<string, mixed> $named): mixed {
  return shape('value' => $named['d'] * 100);
}

function multi_handler_custom_named_args($name, $obj, inout $args, ?dict<string, mixed> $_named) {
  $d = dict["b" => 1, "a" => vec[1, 2, 3]];
  $result = \hphp_invoke_callable_named_args(multi_intercept<>, $args, $d);
  // Ensure that we can still reference $d after 
  var_dump($d["a"]);
  return shape('value' => $result);
}

function free(named int $e) { return $e; }

function free_handler($name, $obj, inout $args, ?dict<string, mixed> $named): mixed {
  return shape('value' => $named['e'] * 200);
}

function main(A $a) :mixed{
  var_dump($a->foo(a=1));
  var_dump($a->bar(1, a=5));
  var_dump($a->baz(c=2));
  var_dump(A::qux(d=4));
  var_dump(free(e=5));
  var_dump($a->multi(a=vec[1], b=9));
}

<<__EntryPoint>>
function main_intercept_methods() :mixed{
  main(new A);
  fb_intercept2_named_args('A::foo', foo_handler<>);
  fb_intercept2_named_args('A::bar', bar_handler<>);
  fb_intercept2_named_args('A::baz', baz_handler<>);
  fb_intercept2_named_args('A::qux', qux_handler<>);
  fb_intercept2_named_args('A::multi', multi_handler_custom_named_args<>);
  fb_intercept2_named_args('free', free_handler<>);
  main(new A);
}
