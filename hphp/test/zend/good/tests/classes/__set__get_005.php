<?hh
class Test
{
    protected $x = varray[];

    function __get($name) {
        echo __METHOD__ . "\n";
        if (isset($this->x[$name])) {
            return $this->x[$name];
        }
        else
        {
            return NULL;
        }
    }

    function __set($name, $val) {
        echo __METHOD__ . "\n";
        $this->x[$name] = $val;
    }
}

class AutoGen
{
    protected $x = varray[];

    function __get($name) {
        echo __METHOD__ . "\n";
        if (!isset($this->x[$name])) {
            $this->x[$name] = new Test();
        }
        return $this->x[$name];
    }

    function __set($name, $val) {
        echo __METHOD__ . "\n";
        $this->x[$name] = $val;
    }
}
<<__EntryPoint>> function main(): void {
$foo = new AutoGen();
$foo->bar->baz = "Check";

var_dump($foo->bar);
var_dump($foo->bar->baz);

echo "===DONE===\n";
}
