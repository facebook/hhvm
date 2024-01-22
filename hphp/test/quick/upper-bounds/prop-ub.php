<?hh

function test($fn) {
  try {
    $fn();
  } catch (Exception $e) {
    echo "Warning: ".$e->getMessage()."\n\n";
  }
}

<<__EntryPoint>>
function main(): mixed {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });

  include "prop-ub-class-1.inc";
  include "prop-type-alias.inc";
  include "prop-ub-class-2.inc";

  test(() ==> new Foo1());
  test(() ==> new Foo2());
  test(() ==> new Foo3());
  test(() ==> new Foo4());
  test(() ==> new Foo5());
  $o = new Foo;
  test(() ==> $o->x = 3.14);
  test(() ==> $o->x = vec[1]);
  test(() ==> $o->y = null);
  test(() ==> $o->y = new Bar);
  test(() ==> $o->y = new NoBar);
  test(() ==> Foo::$sx = null);
  test(() ==> Foo::$sx = 'd');
  test(() ==> Foo::$sy = new Bar);
  test(() ==> Foo::$sy = vec[2]);
  test(() ==> $o->z = 10);
  test(() ==> $o->z = vec[3]);
  test(() ==> $o->w = null);
  test(() ==> $o->v = null); // not enforced
  test(() ==> $o->u = 'a');  // not enforced
}
