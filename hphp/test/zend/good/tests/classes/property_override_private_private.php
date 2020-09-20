<?hh
class A
{
    private $p = "A::p";
    function showA()
    {
        echo $this->p . "\n";
    }
}

class B extends A
{
    private $p = "B::p";
    function showB()
    {
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
