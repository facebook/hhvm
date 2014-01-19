<?php

	trait ATrait
	{
		function setRequired()
		{
			$this->setAttribute();
		}

		abstract function setAttribute();
	}	

	class Base
	{
		function setAttribute() { }
	}

	class MyClass extends Base
	{
		use ATrait;
	}

	$i = new Base();
	$i->setAttribute();

	$t = new MyClass();
	/* setAttribute used to disappear for no good reason. */
	$t->setRequired();
	echo 'DONE';
?>