<?hh
<<__DynamicallyConstructible>>
class Test {
    function say_hello() :mixed{
        echo "Hello world";
    }
}

class Factory {
    public $name = "Test";
    function create() :mixed{
        $obj = new $this->name; /* Parse error */
        return $obj;
    }
}
<<__EntryPoint>> function main(): void {
$factory = new Factory;
$test = $factory->create();
$test->say_hello();
}
