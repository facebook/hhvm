<?hh

class Foo {

  public static int $i = 0;

}

function read_static_prop_via_var_with_read_globals()[read_globals] : void {
  $foo = new Foo();
  $x = readonly $foo::$i; // No error
}

function read_static_prop_via_var()[] : void {
  $foo = new Foo();
  $x = $foo::$i; // Error
}
