<?hh

interface TestInterface {
    public function foo();
    public function bar(array $bar);
}

class Test implements TestInterface {
    public function foo(...$args) {
        echo __METHOD__, "\n";
    }
    public function bar(array $bar, ...$args) {
        echo __METHOD__, "\n";
    }
}
<<__EntryPoint>> function main(): void {
$obj = new Test;
$obj->foo();
$obj->bar(varray[]);
}
