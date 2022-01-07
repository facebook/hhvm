<?hh

class Foo {

  public static int $i = 0;

}

function write_static_prop_via_var_with_globals()[globals] : void {
  $foo = new Foo();
  $foo::$i = 4;
}

function write_static_prop_via_var()[] : void {
  $foo = new Foo();
  $foo::$i = 4; // error
}
