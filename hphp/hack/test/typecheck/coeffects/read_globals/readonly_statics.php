<?hh

class Foo {

  public static int $i = 0;

}

function globals_no_error()[globals] : void {
    $x = Foo::$i; // No error
    Foo::$i = 5; // No error
    $y = readonly Foo::$i; // No error
}

function read_globals_error_on_access()[read_globals] : void {
    $x = Foo::$i; // Readonly error
    Foo::$i = 5; // Globals error
    $y = readonly Foo::$i; // No error
}

function pure_errors_on_read_and_access()[] : void {
    $x = Foo::$i; // ReadGlobals error
    Foo::$i = 5; // Globals error, ReadGlobals error
    $y = readonly Foo::$i; // ReadGlobals error
}
