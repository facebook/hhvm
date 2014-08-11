<?php
class pubf {
	public function f() {}
	static public function s() {}	
}
class subpubf extends pubf {
}

class protf {
	protected function f() {}
	static protected function s() {}	
}
class subprotf extends protf {
}

class privf {
	private function f() {}
	static private function s() {}
}
class subprivf extends privf  {
}

$classes = array("pubf", "subpubf", "protf", "subprotf", 
				 "privf", "subprivf");
foreach($classes as $class) {
	echo "Reflecting on class $class: \n";
	$rc = new ReflectionClass($class);
	var_dump($rc->getMethods());
}

?>
