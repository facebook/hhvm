<?hh

class Foo {
    public static $foo;
}

<<__EntryPoint>>
function main(): mixed {
  Foo::$foo = "foo";
  var_dump(98);
  $a = () ==> Foo::$foo."barff";
  $b = $a();
  var_dump($b);
}
