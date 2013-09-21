<?php
abstract class Singleton
{
	static private $instances = array();
	static private $nextInstanceId = 0;
	private $instanceId = NULL;
	static final public function getInstance()
	{
		$caller = get_called_class();
		if (!isset(self::$instances[$caller])) {
			self::$instances[$caller] = new $caller;
			self::$instances[$caller]->instanceId = self::$nextInstanceId++;
		}
		return self::$instances[$caller];
	}
	public final function getInstanceId()
	{
		return $this->instanceId;
	}
	public final function identify()
	{
		var_dump($this);
	}
}

class Foo extends Singleton {
}

class Bar extends Singleton {
}

class Baz extends Bar {
}

$u = Foo::getInstance();
$v = Bar::getInstance();
$w = Baz::getInstance();

$u->identify();
$v->identify();
$w->identify();

$x = Foo::getInstance();
$y = Bar::getInstance();
$z = Baz::getInstance();

$u->identify();
$v->identify();
$w->identify();
$x->identify();
$y->identify();
$z->identify();
?>
===DONE===