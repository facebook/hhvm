<?hh
class test {
    protected $foo;
    private $bar;
    public $test;

    function foo()
:mixed    {
        $this->bar = 'meuh';
        $this->foo = 'lala';
        $this->test = 'test';

        var_dump(http_build_query($this));
    }
}
<<__EntryPoint>> function main(): void {
$obj = new test();
$obj->foo();
var_dump(http_build_query($obj));
}
