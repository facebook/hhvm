<?hh

class Foo
{
    protected static $fooStatic = 'foo';
    protected $foo = 'foo';
}
<<__EntryPoint>> function main(): void {
$class = new ReflectionClass('Foo');

var_dump($class->getDefaultProperties());

echo "Done\n";
}
