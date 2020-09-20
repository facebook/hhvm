<?hh

class MyClass {
  public static $x;
  public static function foo() {
    echo "in static method foo()\n";
  }
}

function clsFact() {
  return new MyClass();
}

// Cls :: [ String ] -> [ Class ]
<<__EntryPoint>> function main(): void {
MyClass::$x = 1;
// Any way to coerce the [ Obj ] -> [ Class ] variety?
$foo = new MyClass();
$foo::$x = 1;

// ClsH
$refs = varray[];
$refs[] = clsFact();
$refs[] = 'MyClass';
$s = 'MyClass';

foreach($refs as $r) {
  $r::foo();
}
}
