<?hh

class Foo {
  public int $i = 0;
}

function read_globals_readonly_error()[read_globals] : void {
  $x = Foo::$i; // Readonly error
}
