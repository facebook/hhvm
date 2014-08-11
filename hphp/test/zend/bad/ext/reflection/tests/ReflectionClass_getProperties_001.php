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
	static private $s;
}
class subprivf extends privf  {
}

$classes = array("pubf", "subpubf", "protf", "subprotf", 
				 "privf", "subprivf");
foreach($classes as $class) {
	echo "Reflecting on class $class: \n";
	$rc = new ReflectionClass($class);
	var_dump($rc->getProperties());
}

?>
