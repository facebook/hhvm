<?hh
class A
{
    private function Test()
    {
        echo 'Hello from '.get_class($this)."\n";
    }

    public function call($method, $args = varray[])
    {
        $this->Test();
        $this->$method();
        $this->$method();
    }
}

class B extends A
{
    protected function Test()
    {
        echo 'Overridden hello from '.get_class($this)."\n";
    }
}
<<__EntryPoint>> function main(): void {
$a = new A;
$b = new B;

$a->call('Test');
$b->call('Test');
}
