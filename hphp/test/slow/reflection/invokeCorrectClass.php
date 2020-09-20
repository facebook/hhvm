<?hh

class Foo {
  public static function bar() {
    echo static::class."\n";
  }
  public function boo($var) {
    var_dump($var);
    echo static::class."\n";
 }
}
class Baz extends Foo { }


<<__EntryPoint>>
function main_invoke_correct_class() {
$class = new ReflectionClass('Baz');
$bar_method = $class->getMethod('bar');
$bar_method->invoke(null); // the correct answer is 'Baz'
$boo_method = $class->getMethod('boo');
$boo_method->invokeArgs(new Baz(), varray[true]);

$standalone_bar = new ReflectionMethod('Baz', 'bar');
$standalone_bar->invoke(null);
$standalone_boo = new ReflectionMethod('Baz', 'boo');
$standalone_boo->invokeArgs(new Baz(), varray[true]);
}
