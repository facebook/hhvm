<?php
class C {
	// These values get implicitly converted to objects
	public $boolFalse = false;
	public $emptyString = '';
	public $null = null;

	// These values do not get implicitly converted to objects
	public $boolTrue = true;
	public $nonEmptyString = 'hello';
	public $intZero = 0;
}

$c = new C;
foreach($c as $name => $value) {
	echo "\n\n---( \$c->$name )---";
	echo "\n  --> Attempting implicit conversion to object using increment...\n";
	$c->$name->prop++;
	$c->$name = $value; // reset value in case implicit conversion was successful

	echo "\n  --> Attempting implicit conversion to object using assignment...\n";
	$c->$name->prop = "Implicit instantiation!";
	$c->$name = $value; // reset value in case implicit conversion was successful

	echo "\n  --> Attempting implicit conversion to object using combined assignment...\n";
	$c->$name->prop .= " Implicit instantiation!";
}

echo "\n\n\n --> Resulting object:";
var_dump($c);

?>