<?php
class A implements ArrayAccess {
	private $a = [];
	function offsetGet($offset) {
		return $this->a[$offset];
	}
        function offsetSet($offset, $value) {
                $this->a[$offset] = $value;
        }
        function offsetExists($offset) {
                isset($this->a[$offset]);
        }
        function offsetUnset($offset) {
                unset($this->a[$offset]);
        }
}

$obj = new ArrayObject(["a" => 1]);
$obj["a"] .= "test";
var_dump($obj["a"]);

$obj = new A;
$obj["a"] = 1;
$obj["a"] .= "test";
var_dump($obj["a"]);
