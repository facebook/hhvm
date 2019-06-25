<?hh

class SomeClassName
{
    public $var = 'bla';
}

function test (OtherClassName $object) { }

function __autoload($class)
{
    var_dump("__autload($class)");
}
<<__EntryPoint>> function main(): void {
$obj = new SomeClassName;
test($obj);

echo "Done\n";
}
