<?php

class array_iterator implements IteratorAggregate {
        public function getIterator() {
                return array('foo', 'bar');     
        }
}

$obj = new array_iterator;

try
{
	foreach ($obj as $property => $value)
	{
		var_dump($value);
	}
}
catch(Exception $e)
{
	echo $e->getMessage() . "\n";
}
?>
===DONE===