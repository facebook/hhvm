<?hh

/*
 * Test intercepts where callsites have already been bound to the
 * pre-intercept function.
 */

function foo($i = 10) :mixed{
  var_dump(__METHOD__);
}

function bar($_1, $_2, inout $_3) :mixed{
  var_dump(__METHOD__);
  return shape('value' => null);
}

function test() :mixed{
  foo();
}

class C {
  function snoot() :mixed{
    var_dump(__METHOD__);
  }
}


function swizzle($name, $obj, inout $args) :mixed{
  var_dump($name, $obj, $args);
  return shape();
}

<<__EntryPoint>> function main(): void {
  $c = new C();
  for ($i = 0; $i < 3; $i++) {
    test();
    foo();
    $c->snoot();
    if ($i == 1) {
      fb_intercept2('foo', 'bar');
      fb_intercept2('C::snoot', 'swizzle');
    }
  }
}
