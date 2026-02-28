<?hh
class ExampleParent {
    private $hello_world = "hello foo\n";
    public function foo() :mixed{
           echo $this->hello_world;
    }
}

class Example extends ExampleParent {
    use ExampleTrait;
}

trait ExampleTrait {
    /**
     *
     */
    private $hello_world = "hello bar\n";
    /**
     *
     */
    public $prop = "ops";
    public function bar() :mixed{
        echo $this->hello_world;
    }
}
<<__EntryPoint>> function main(): void {
$x = new Example();
$x->foo();
$x->bar();
}
