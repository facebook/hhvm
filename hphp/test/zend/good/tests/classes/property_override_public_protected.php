<?hh
class A
{
    public $p = "A::p";
    function showA()
:mixed    {
        echo $this->p . "\n";
    }
}

class B extends A
{
    protected $p = "B::p";
    function showB()
:mixed    {
        echo $this->p . "\n";
    }
}

<<__EntryPoint>> function main(): void {
$a = new A;
$a->showA();

$b = new B;
$b->showA();
$b->showB();
}
