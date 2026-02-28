<?hh

class MyClass {
  public function __construct() {
    echo "ctor\n";
  }
  public function method() :mixed{
    echo "method\n";
  }

  const JUNK = "ASD";

  public static $SomeProp = null;
}
type Yeah = MyClass;

function target(Yeah $x): void {
  $x->method();
}

function hinting(): void {
  $x = new MyClass();
  target($x);
}


<<__EntryPoint>>
function main_typedef_class() :mixed{
hinting();
}
