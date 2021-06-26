<?hh

class Foo {
    protected static $className = 'Foo';
    <<__DynamicallyCallable>> public static function bar() {
        echo static::$className . "::bar\n";
    }
    public function __construct() {
        echo static::$className . "::__construct\n";
    }
}

class FooChild extends Foo {
    protected static $className = 'FooChild';
}
<<__EntryPoint>> function main(): void {
register_shutdown_function(varray['Foo', 'bar']);
register_shutdown_function(varray['FooChild', 'bar']);

$foo = new Foo();
$fooChild = new FooChild();
unset($foo);
unset($fooChild);
}
