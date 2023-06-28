<?hh

trait T {
  protected $foo;
}

class X {
  use T;
}

class Y extends X {
  public static $foo;
}


<<__EntryPoint>>
function main_static_redecl_from_trait() :mixed{
echo "Ok\n";
}
