<?php
class pubf {
	public $a;
	static public $s;
}
class subpubf extends pubf {
}

class protf {
	protected $a;
	static protected $s;
}
class subprotf extends protf {
}

class privf {
	private $a;
	static protected $s;
}
class subprivf extends privf  {
}

$classes = array("pubf", "subpubf", "protf", "subprotf", 
				 "privf", "subprivf");
foreach($classes as $class) {
	echo "Reflecting on class $class: \n";
	$rc = new ReflectionClass($class);
	try {
		echo "  --> Check for s: ";
		var_dump($rc->getProperty("s"));
	} catch (exception $e) {
		echo $e->getMessage() . "\n";	
	}
	try {
		echo "  --> Check for a: ";
		var_dump($rc->getProperty("a"));
	} catch (exception $e) {
		echo $e->getMessage() . "\n";	
	}	
	try {
		echo "  --> Check for A: ";
		var_dump($rc->getProperty("A"));
	} catch (exception $e) {
		echo $e->getMessage() . "\n";	
	}
	try {
		echo "  --> Check for doesntExist: ";
		var_dump($rc->getProperty("doesntExist"));
	} catch (exception $e) {
		echo $e->getMessage() . "\n";	
	}

}
?>
