<?hh

class Foo {

  public static function static_method()[] : bool {
    return true;
  }
}

function call_static_method_via_classname()[] : void {
  Foo::static_method(); // No error
}

function call_static_method_via_var(Foo $foo)[] : void {
  $foo::static_method(); // No error
}

<<__EntryPoint>>
function main(): void {
  $foo = new Foo();

  call_static_method_via_classname();
  call_static_method_via_var($foo);

  echo "Done\n";

}
