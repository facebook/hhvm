<?php
class privateCtorOld {
	private function privateCtorOld() {}
	public static function reflectionObjectFactory() {
		return new ReflectionObject(new self);
	}	
}
$reflectionObject =  privateCtorOld::reflectionObjectFactory();

var_dump($reflectionObject->IsInstantiable('X'));
var_dump($reflectionObject->IsInstantiable(0, null));

?>
