<?hh

class Foo
{
    private $name = 'testBAR';

    public function testBAR()
:mixed    {
        try
        {
            $class  = new ReflectionClass($this);
            var_dump($this->name);
            $method = $class->getMethod($this->name);
            var_dump($this->name);
        }

        catch (Exception $e) {}
    }
}
<<__EntryPoint>> function main(): void {
$foo = new Foo;
$foo->testBAR();
echo "===DONE===\n";
}
