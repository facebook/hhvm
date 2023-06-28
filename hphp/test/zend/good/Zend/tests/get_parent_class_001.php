<?hh

interface ITest {
    function foo():mixed;
}

abstract class bar implements ITest {
    public function foo() :mixed{
        var_dump(get_parent_class());
    }
}

class foo extends bar {
    public function __construct() {
        var_dump(get_parent_class());
    }
}
<<__EntryPoint>> function main(): void {
$a = new foo;
$a->foo();
}
