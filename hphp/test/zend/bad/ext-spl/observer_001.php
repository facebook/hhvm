<?php

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
	protected $observers = array();

	function __construct($name = 'sub')
	{
		$this->name = '$' . $name;
	}

    function attach(SplObserver $observer)
    {
    	echo '$sub->' . __METHOD__ . '(' . $observer->getName() . ");\n";
    	if (!in_array($observer, $this->observers))
    	{
	    	$this->observers[] = $observer;
	    }
    }

    function detach(SplObserver $observer)
    {
    	echo '$sub->' . __METHOD__ . '(' . $observer->getName() . ");\n";
    	$idx = array_search($observer, $this->observers);
    	if ($idx !== false)
    	{
    		unset($this->observers[$idx]);
    	}
    }

    function notify()
    {
    	echo '$sub->' . __METHOD__ . "();\n";
    	foreach($this->observers as $observer)
    	{
    		$observer->update($this);
    	}
    }

	function getName()
	{
		return $this->name;
	}
}

$sub = new SubjectImpl;

$ob1 = new ObserverImpl("ob1");
$ob2 = new ObserverImpl("ob2");
$ob3 = new ObserverImpl("ob3");

$sub->attach($ob1);
$sub->attach($ob1);
$sub->attach($ob2);
$sub->attach($ob3);

$sub->notify();

$sub->detach($ob3);

$sub->notify();

$sub->detach($ob2);
$sub->detach($ob1);

$sub->notify();

$sub->attach($ob3);

$sub->notify();
?>
===DONE===