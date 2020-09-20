<?hh

class A
{
    public $a = varray[];

    public function __construct()
    {
        $this->a[] = new B(1);
        $this->a[] = new B(2);
    }
}

class B implements Serializable
{
    public $b;

    public function __construct($c)
    {
        $this->b = new C($c);
    }

    public function serialize()
    {
        return serialize(clone $this->b);
    }

    public function unserialize($data)
    {
        $this->b = unserialize($data);
    }
}

class C
{
    public $c;

    public function __construct($c)
    {
        $this->c = $c;
    }
}
<<__EntryPoint>> function main(): void {
echo "Test\n";

$a = unserialize(serialize(new A()));

print $a->a[0]->b->c . "\n";
print $a->a[1]->b->c . "\n";
echo "Done";
}
