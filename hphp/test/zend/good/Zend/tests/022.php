<?hh

abstract class Base
{
    abstract function someMethod($param):mixed;
}

class Ext extends Base
{
    function someMethod($param = "default")
:mixed    {
        echo $param, "\n";
    }
}
<<__EntryPoint>> function main(): void {
$a = new Ext();
$a->someMethod("foo");
$a->someMethod();
}
