<?php

class NonSerializingTest
{
	public $data;

	public function __construct($data)
	{
		$this->data = $data;
	}
}

class SerializingTest extends NonSerializingTest implements JsonSerializable
{
	public function jsonSerialize()
	{
		return $this->data;
	}
}

class ValueSerializingTest extends SerializingTest
{
	public function jsonSerialize()
	{
		return array_values(is_array($this->data) ? $this->data : get_object_vars($this->data));
	}
}

class SelfSerializingTest extends SerializingTest
{
	public function jsonSerialize()
	{
		return $this;
	}
}

$adata = array(
	'str'	=> 'foo',
	'int'	=> 1,
	'float'	=> 2.3,
	'bool'	=> false,
	'nil'	=> null,
	'arr'	=> array(1,2,3),
	'obj'	=> new StdClass,
);

$ndata = array_values($adata);

$odata = (object)$adata;

foreach(array('NonSerializingTest','SerializingTest','ValueSerializingTest','SelfSerializingTest') as $class) {
	echo "==$class==\n";
	echo json_encode(new $class($adata)), "\n";
	echo json_encode(new $class($ndata)), "\n";
	echo json_encode(new $class($odata)), "\n";
}