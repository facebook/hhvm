<?hh

class Foo {

  public static int $i = 0;

}

function read_static_prop_via_classname_with_read_globals()[read_globals] : void {
  $x = readonly Foo::$i; // No error
}

function read_static_prop_via_classname()[] : void {
  $x = Foo::$i; // Error
}
