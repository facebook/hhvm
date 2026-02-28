<?hh

trait ATrait
{
    function setRequired()
:mixed    {
        $this->setAttribute();
    }

    abstract function setAttribute():mixed;
}

class Base
{
    function setAttribute() :mixed{ }
}

class MyClass extends Base
{
    use ATrait;
}
<<__EntryPoint>> function main(): void {
$i = new Base();
$i->setAttribute();

$t = new MyClass();
/* setAttribute used to disappear for no good reason. */
$t->setRequired();
echo 'DONE';
}
