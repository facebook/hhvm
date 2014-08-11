<?php

class noCtor {
	public static function reflectionObjectFactory() {
		return new ReflectionObject(new self);
	}	
}

class publicCtorNew {
	public function __construct() {}
	public static function reflectionObjectFactory() {
		return new ReflectionObject(new self);
	}	
}

class protectedCtorNew {
	protected function __construct() {}
	public static function reflectionObjectFactory() {
		return new ReflectionObject(new self);
	}	
}

class privateCtorNew {
	private function __construct() {}
	public static function reflectionObjectFactory() {
		return new ReflectionObject(new self);
	}	
}

class publicCtorOld {
	public function publicCtorOld() {}
	public static function reflectionObjectFactory() {
		return new ReflectionObject(new self);
	}	
}

class protectedCtorOld {
	protected function protectedCtorOld() {}
	public static function reflectionObjectFactory() {
		return new ReflectionObject(new self);
	}	
}

class privateCtorOld {
	private function privateCtorOld() {}
	public static function reflectionObjectFactory() {
		return new ReflectionObject(new self);
	}	
}


$reflectionObjects = array(
		noCtor::reflectionObjectFactory(),
		publicCtorNew::reflectionObjectFactory(),
		protectedCtorNew::reflectionObjectFactory(),
		privateCtorNew::reflectionObjectFactory(),
		publicCtorOld::reflectionObjectFactory(), 
		protectedCtorOld::reflectionObjectFactory(),
		privateCtorOld::reflectionObjectFactory()
	);

foreach($reflectionObjects  as $reflectionObject ) {
	$name = $reflectionObject->getName();
	echo "Is $name instantiable? ";
	var_dump($reflectionObject->IsInstantiable()); 
}
?>
