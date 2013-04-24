<?php

class MyObjectStorage extends SplObjectStorage
{
	function rewind()
	{
		echo __METHOD__ . "()\n";
		parent::rewind();
	}

	function valid()
	{
		echo __METHOD__ . "(" . (parent::valid() ? 1 : 0) . ")\n";
		return parent::valid();
	}

	function key()
	{
		echo __METHOD__ . "(" . parent::key() . ")\n";
		return parent::key();
	}

	function current()
	{
		echo __METHOD__ . "(" . parent::current()->getName() . ")\n";
		return parent::current();
	}

	function next()
	{
		echo __METHOD__ . "()\n";
		parent::next();
	}
}

class ObserverImpl implements SplObserver
{
	protected $name = '';

	function __construct($name = 'obj')
	{
		$this->name = '$' . $name;
	}

	function update(SplSubject $subject)
	{
		echo $this->name . '->' . __METHOD__ . '(' . $subject->getName() . ");\n";
	}
	
	function getName()
	{
		return $this->name;
	}
}

class SubjectImpl implements SplSubject
{
	protected $name = '';
	protected $observers;

	function __construct($name = 'sub')
	{
		$this->observers = new MyObjectStorage;
		$this->name = '$' . $name;
	}

	function attach(SplObserver $observer)
	{
		echo $this->name . '->' . __METHOD__ . '(' . $observer->getName() . ");\n";
		$this->observers->attach($observer);
	}
	
	function detach(SplObserver $observer)
	{
		echo $this->name . '->' . __METHOD__ . '(' . $observer->getName() . ");\n";
		$this->observers->detach($observer);
	}
	
	function count()
	{
		return $this->observers->count();
	}
	
	function notify()
	{
		echo $this->name . '->' . __METHOD__ . "();\n";
		foreach($this->observers as $key => $observer)
		{
			$observer->update($this);
		}
	}

	function getName()
	{
		return $this->name;
	}
	
	function contains($obj)
	{
		return $this->observers->contains($obj);
	}
}

$sub = new SubjectImpl;

$ob1 = new ObserverImpl("ob1");
$ob2 = new ObserverImpl("ob2");
$ob3 = new ObserverImpl("ob3");

var_dump($sub->contains($ob1));
$sub->attach($ob1);
var_dump($sub->contains($ob1));
$sub->attach($ob1);
$sub->attach($ob2);
$sub->attach($ob3);
var_dump($sub->count());

$sub->notify();

$sub->detach($ob3);
var_dump($sub->count());

$sub->notify();

$sub->detach($ob2);
$sub->detach($ob1);
var_dump($sub->count());

$sub->notify();

$sub->attach($ob3);
var_dump($sub->count());

$sub->notify();

?>
===DONE===
<?php exit(0); ?>