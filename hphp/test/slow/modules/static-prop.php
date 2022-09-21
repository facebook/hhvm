<?hh
<<__EntryPoint>>
function foo(): void {
  include "instance-properties.inc";
    ok();
    $x = Foo::$y; // error
    var_dump($x);
    Foo::$y = 7; // error
    Foo::$y++; // error
    Foo::$y--; // error
    Foo::$sv[]  = 5; // error
    $z = Foo::$sv[0]; // error
    var_dump($z);
    isset(Foo::$sv); // error
    isset(Foo::$sv[0]); // error

}
