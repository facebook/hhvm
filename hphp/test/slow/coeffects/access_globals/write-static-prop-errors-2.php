<?hh

class Foo {

  public static int $i = 0;

}

function write_static_prop_via_classname_with_globals()[globals] : void {
  Foo::$i = 4;
}

function write_static_prop_via_classname()[] : void {
  Foo::$i = 4; // error
}
