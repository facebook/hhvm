<?hh

error_reporting(-1);

class Foo {
  static $bar = darray['baz' => 'myfunc'];
}

function myfunc() {
  return 'quux';
}

var_dump(Foo::$bar['baz']());
