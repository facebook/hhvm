<?hh

class SomeClassName
{
    public $var = 'bla';
}

function test (OtherClassName $object) :mixed{ }

<<__EntryPoint>> function main(): void {
$obj = new SomeClassName;
test($obj);

echo "Done\n";
}
