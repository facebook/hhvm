<?hh
class A {}

class B
{
    public function go()
:mixed    {
        $this->foo = 'bar';
        echo A::$this->foo; // should not output 'bar'
    }
}
<<__EntryPoint>> function main(): void {
$obj = new B();
$obj->go();
}
