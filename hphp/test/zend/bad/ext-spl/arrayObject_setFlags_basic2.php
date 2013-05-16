<?php
class C extends ArrayObject {
	private $x = 'secret';
	
	static function go($c) {
	  var_dump($c->x);
	}
}	

$c = new C(array('x'=>'public'));

$c->setFlags(ArrayObject::ARRAY_AS_PROPS);
C::go($c);
var_dump($c->x);


$c->setFlags(0);
C::go($c);
var_dump($c->x);
?>