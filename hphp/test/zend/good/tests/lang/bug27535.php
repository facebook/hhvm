<?hh

class Class1
{
    public $_Class2_obj;
}

class Class2
{
    public $storage = '';

    function Class2()
:mixed    {
        $this->storage = new Class1();

        $this->storage->_Class2_obj = $this;
    }
}
<<__EntryPoint>> function main(): void {
$foo = new Class2();
echo "Alive!";
}
