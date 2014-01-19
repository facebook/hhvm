<?php

class FooBar implements ArrayAccess {
	private $array = array();

	public function offsetExists($index) {
		return isset($this->array[$index]);
	}

	public function offsetGet($index) {
		return $this->array[$index];
	}

	public function offsetSet($index, $value) {
		echo __METHOD__ . "($index, $value)\n";
		$this->array[$index] = $value;
	}

	public function offsetUnset($index) {
		throw new Exception('FAIL');
		unset($this->array[$index]);
	}

}

$i = 0; $j = 0;
$foo = new FooBar();
$foo[$j++] = $i++;
$foo[$j++] = $i++;
$foo[$j++] = $i++;
try
{
	unset($foo[1]);
}
catch (Exception $e)
{
	echo "CAUGHT: " . $e->getMessage() . "\n";
}

print_R($foo);
?>
===DONE===