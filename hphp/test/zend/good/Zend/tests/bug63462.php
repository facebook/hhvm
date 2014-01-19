<?php
class Test {
	public    $publicProperty;
	protected $protectedProperty;
	private   $privateProperty;

	public function __construct() {
		unset(
			$this->publicProperty, 
			$this->protectedProperty, 
			$this->privateProperty
		);
	}

	function __get($name) {
		echo '__get ' . $name . "\n";
		return $this->$name;
	}

	function __set($name, $value) {
		echo '__set ' . $name . "\n";
		$this->$name = $value;
	}

	function __isset($name) {
		echo '__isset ' . $name . "\n";
		return isset($this->$name);
	}
}

$test = new Test();

$test->nonExisting;
$test->publicProperty;
$test->protectedProperty;
$test->privateProperty;
isset($test->nonExisting);
isset($test->publicProperty);
isset($test->protectedProperty);
isset($test->privateProperty);
$test->nonExisting       = 'value';
$test->publicProperty	 = 'value';
$test->protectedProperty = 'value';
$test->privateProperty   = 'value';

?>
