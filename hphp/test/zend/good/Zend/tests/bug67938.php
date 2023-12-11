<?hh

interface TestInterface {
    public function foo():mixed;
    public function bar(varray $bar):mixed;
}

class Test implements TestInterface {
    public function foo(...$args) :mixed{
        echo __METHOD__, "\n";
    }
    public function bar(varray $bar, ...$args) :mixed{
        echo __METHOD__, "\n";
    }
}
<<__EntryPoint>> function main(): void {
$obj = new Test;
$obj->foo();
$obj->bar(vec[]);
}
