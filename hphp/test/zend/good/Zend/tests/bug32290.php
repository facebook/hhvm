<?hh

class TestA
{
    public function doSomething($i)
:mixed    {
        echo __METHOD__ . "($i)\n";
        return --$i;
    }

    public function doSomethingThis($i)
:mixed    {
        echo __METHOD__ . "($i)\n";
        return --$i;
    }

    public function doSomethingParent($i)
:mixed    {
        echo __METHOD__ . "($i)\n";
        return --$i;
    }

    public function doSomethingParentThis($i)
:mixed    {
        echo __METHOD__ . "($i)\n";
        return --$i;
    }

    public static function doSomethingStatic($i)
:mixed    {
        echo __METHOD__ . "($i)\n";
        return --$i;
    }
}

class TestB extends TestA
{
    public function doSomething($i)
:mixed    {
        echo __METHOD__ . "($i)\n";
        $i++;
        if ($i >= 5) return 5;
        $x = vec["TestA", "doSomething"];
        return $x($i);
    }

    public function doSomethingThis($i)
:mixed    {
        echo __METHOD__ . "($i)\n";
        $i++;
        if ($i >= 5) return 5;
        $x = vec[$this, "TestA::doSomethingThis"];
        return $x($i);
    }

    public function doSomethingParent($i)
:mixed    {
        echo __METHOD__ . "($i)\n";
        $i++;
        if ($i >= 5) return 5;
        $x = vec["parent", "doSomethingParent"];
        return $x($i);
    }

    public function doSomethingParentThis($i)
:mixed    {
        echo __METHOD__ . "($i)\n";
        $i++;
        if ($i >= 5) return 5;
        $x = vec[$this, "parent::doSomethingParentThis"];
        return $x($i);
    }

    public static function doSomethingStatic($i)
:mixed    {
        echo __METHOD__ . "($i)\n";
        $i++;
        if ($i >= 5) return 5;
        $x = vec["TestA", "doSomethingStatic"];
        return $x($i);
    }
}
<<__EntryPoint>> function main(): void {
$x = new TestB();
echo "===A===\n";
var_dump($x->doSomething(1));
echo "\n===B===\n";
var_dump($x->doSomethingThis(1));
echo "\n===C===\n";
var_dump($x->doSomethingParent(1));
echo "\n===D===\n";
var_dump($x->doSomethingParentThis(1));

echo "===DONE===\n";
}
