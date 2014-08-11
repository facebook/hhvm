<?php
class noCtor {
}

class publicCtorNew {
	public function __construct() {}
}

class protectedCtorNew {
	protected function __construct() {}
}

class privateCtorNew {
	private function __construct() {}
}

class publicCtorOld {
	public function publicCtorOld() {}
}

class protectedCtorOld {
	protected function protectedCtorOld() {}
}

class privateCtorOld {
	private function privateCtorOld() {}
}


$classes = array("noCtor", "publicCtorNew", "protectedCtorNew", "privateCtorNew",
				 	"publicCtorOld", "protectedCtorOld", "privateCtorOld");

foreach($classes  as $class ) {
	$reflectionClass = new ReflectionClass($class);
	echo "Is $class instantiable?  ";
	var_dump($reflectionClass->IsInstantiable()); 
}

?>
