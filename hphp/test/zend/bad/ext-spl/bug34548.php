<?php

class Collection extends ArrayObject
{
	public function add($dataArray)
	{
		foreach($dataArray as $value) $this->append($value);
	}

	public function offsetSet($index, $value)
	{
		parent::offsetSet($index, $value);
	}
}

$data1=array('one', 'two', 'three');
$data2=array('four', 'five');

$foo=new Collection($data1);
$foo->add($data2);

print_r($foo->getArrayCopy());

echo "Done\n";
?>