<?hh

class Foo {

  public static int $i = 0;

  public function __construct()[] : void {
    $x = self::$i; // Error
  }

}

function read_static_prop_via_classname()[] : void {
  $x = Foo::$i; // Error
}

function read_static_prop_via_classname_with_read_globals()[read_globals] : void {
  $x = Foo::$i; // No error
}

function read_static_prop_via_var()[] : void {
  $foo = new Foo();
  $x = $foo::$i; // Error
}

function read_static_prop_via_var_with_read_globals()[read_globals] : void {
  $foo = new Foo();
  $x = $foo::$i; // No error
}
