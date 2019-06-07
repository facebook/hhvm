<?hh
class A
{
    public $p = "A::p";
    function showA()
    {
        echo $this->p . "\n";
    }
}

class B extends A
{
    protected $p = "B::p";
    function showB()
    {
        echo $this->p . "\n";
    }
}

<<__EntryPoint>> function main() {
$a = new A;
$a->showA();

$b = new B;
$b->showA();
$b->showB();
}
