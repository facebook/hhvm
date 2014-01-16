<?php

class TestA
{
	public function doSomething($i)
	{
		echo __METHOD__ . "($i)\n";
		return --$i;
	}

	public function doSomethingThis($i)
	{
		echo __METHOD__ . "($i)\n";
		return --$i;
	}

	public function doSomethingParent($i)
	{
		echo __METHOD__ . "($i)\n";
		return --$i;
	}

	public function doSomethingParentThis($i)
	{
		echo __METHOD__ . "($i)\n";
		return --$i;
	}

	public static function doSomethingStatic($i)
	{
		echo __METHOD__ . "($i)\n";
		return --$i;
	}
}

class TestB extends TestA
{
	public function doSomething($i)
	{
		echo __METHOD__ . "($i)\n";
		$i++;
		if ($i >= 5) return 5;
		return call_user_func_array(array("TestA", "doSomething"), array($i));
	}

	public function doSomethingThis($i)
	{
		echo __METHOD__ . "($i)\n";
		$i++;
		if ($i >= 5) return 5;
		return call_user_func_array(array($this, "TestA::doSomethingThis"), array($i));
	}

	public function doSomethingParent($i)
	{
		echo __METHOD__ . "($i)\n";
		$i++;
		if ($i >= 5) return 5;
		return call_user_func_array(array("parent", "doSomethingParent"), array($i));
	}

	public function doSomethingParentThis($i)
	{
		echo __METHOD__ . "($i)\n";
		$i++;
		if ($i >= 5) return 5;
		return call_user_func_array(array($this, "parent::doSomethingParentThis"), array($i));
	}

	public static function doSomethingStatic($i)
	{
		echo __METHOD__ . "($i)\n";
		$i++;
		if ($i >= 5) return 5;
		return call_user_func_array(array("TestA", "doSomethingStatic"), array($i));
	}
}

$x = new TestB();
echo "===A===\n";
var_dump($x->doSomething(1));
echo "\n===B===\n";
var_dump($x->doSomethingThis(1));
echo "\n===C===\n";
var_dump($x->doSomethingParent(1));
echo "\n===D===\n";
var_dump($x->doSomethingParentThis(1));
echo "\n===E===\n";
var_dump($x->doSomethingStatic(1));

?>
===DONE===
<?php exit(0); ?>