<?php

class ArrayAccessImpl implements ArrayAccess {
	private $data = array();

	public function offsetUnset($index) {}

	public function offsetSet($index, $value) {
		$this->data[$index] = $value;
	}

	public function offsetGet($index) {
		return $this->data[$index];
	}

	public function offsetExists($index) {
		return isset($this->data[$index]);
	}
}

$data = new ArrayAccessImpl();
$test = 'some data';
$data['element'] = NULL; // prevent notice
$data['element'] = &$test;

?>
===DONE===
<?php exit(0); ?>
