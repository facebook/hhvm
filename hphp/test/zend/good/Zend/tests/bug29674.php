<?hh

class BaseClass
{
    private $private_base = "Base";

    function printVars ()
    {
        var_dump($this->private_base);
        var_dump($this->private_child);
    }
}

class ChildClass extends BaseClass
{
    private $private_child = "Child";
}
<<__EntryPoint>> function main(): void {
echo "===BASE===\n";
$obj = new BaseClass;
$obj->printVars();

echo "===CHILD===\n";
$obj = new ChildClass;
$obj->printVars();

echo "===DONE===\n";
}
