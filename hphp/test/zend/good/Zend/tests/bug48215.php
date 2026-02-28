<?hh

class A
{
    public function __construct() {
        echo __METHOD__ . "\n";
    }
    protected function A()
:mixed    {
        echo __METHOD__ . "\n";
    }
}

class B extends A
{
    public function __construct() {
        echo __METHOD__ . "\n";
        parent::__construct();
    }
    public function A()
:mixed    {
        echo __METHOD__ . "\n";
        parent::A();
    }
}

<<__EntryPoint>> function main(): void {
$b = new B();
$b->A();
echo "===DONE===\n";
}
