<?php
class C {
	private $p = 1;
}

class D extends C{
}

$Obj = new D;
$Obj->p = 'value';
ReflectionObject::export($Obj)
?>
