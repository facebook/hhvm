<?hh
class A
{
    private $c = "A's c";
}

class B extends A
{
    private $c = "B's c";

    public function go()
:mixed    {
        foreach ($this as $key => $val)
        {
            echo "$key => $val\n";
        }
    }
}
<<__EntryPoint>> function main(): void {
$x = new B;
$x->go();
}
