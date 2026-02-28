<?hh

class MyClass {
  public static $x;
  public static function foo() :mixed{
    echo "in static method foo()\n";
  }
}

function clsFact() :mixed{
  return new MyClass();
}

// Cls :: [ String ] -> [ Class ]
<<__EntryPoint>> function main(): void {
MyClass::$x = 1;
// Any way to coerce the [ Obj ] -> [ Class ] variety?
$foo = new MyClass();
$foo::$x = 1;

// ClsH
$refs = vec[];
$refs[] = clsFact();
$refs[] = 'MyClass';
$s = 'MyClass';

foreach($refs as $r) {
  $r::foo();
}
}
