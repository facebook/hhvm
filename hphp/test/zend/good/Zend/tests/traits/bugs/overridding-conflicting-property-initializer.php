<?hh

trait foo
{
    public $zoo = 'foo::zoo';
}

class baz
{
    use foo;
    public $zoo = 'baz::zoo';
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$obj = new baz();
echo $obj->zoo, "\n";
}
