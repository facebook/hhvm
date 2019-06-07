<?hh

class Foo extends OAuth
{
    protected $foo = 'bar';

    public function getFoo()
    {
        return $this->foo;
    }
}
<<__EntryPoint>> function main() {
$foo = new Foo('key', 'secret');
var_dump($foo->getFoo());
}
