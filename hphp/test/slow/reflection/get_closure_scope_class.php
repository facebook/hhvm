<?hh

class C {
  public function __construct() {
    $fn = function () { return get_called_class(); };
    $res = (new ReflectionFunction($fn))->getClosureScopeClass();

    var_dump(get_class($res));
    var_dump($res->getName());
  }

  public static function s_fn() {
    $fn = function () { return get_called_class(); };
    $res = (new ReflectionFunction($fn))->getClosureScopeClass();

    var_dump(get_class($res));
    var_dump($res->getName());
  }
}

class D extends C {
  public function __construct() {
    static::s_fn();
  }
}


<<__EntryPoint>>
function main_get_closure_scope_class() {
$fn = function () { return get_called_class(); };
var_dump((new ReflectionFunction($fn))->getClosureScopeClass());

$c = new C;
C::s_fn();
$d = new D;
}
