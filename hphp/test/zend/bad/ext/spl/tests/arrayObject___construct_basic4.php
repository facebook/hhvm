<?php
class C {
	public $prop = 'C::prop.orig';
}

class MyArrayObject extends ArrayObject {
	public $prop = 'MyArrayObject::prop.orig';
}	

echo "\n--> Access prop on instance of ArrayObject with ArrayObject::ARRAY_AS_PROPS:\n";
$c = new C;
$ao = new ArrayObject($c, ArrayObject::ARRAY_AS_PROPS);
testAccess($c, $ao);

echo "\n--> Access prop on instance of MyArrayObject with ArrayObject::ARRAY_AS_PROPS:\n";
$c = new C;
$ao = new MyArrayObject($c, ArrayObject::ARRAY_AS_PROPS);
testAccess($c, $ao);

function testAccess($c, $ao) {
	echo "  - Iteration:\n";
	foreach ($ao as $key=>$value) {
		echo "      $key=>$value\n";
	}

	echo "  - Read:\n";
	@var_dump($ao->prop, $ao['prop']);
	
	echo "  - Write:\n";
	$ao->prop = 'changed1';
	$ao['prop'] = 'changed2';
	var_dump($ao->prop, $ao['prop']);
	
	echo "  - Isset:\n";
	var_dump(isset($ao->prop), isset($ao['prop']));
	
	echo "  - Unset:\n";
	unset($ao->prop);
	unset($ao['prop']);
	var_dump($ao->prop, $ao['prop']);
	
	echo "  - After:\n";
	var_dump($ao, $c);
}
?>