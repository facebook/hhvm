<?hh

trait T {
  protected $foo;
}

class X {
  use T;
}

class Y extends X {
  static $foo;
}


<<__EntryPoint>>
function main_static_redecl_from_trait() {
echo "Ok\n";
}
