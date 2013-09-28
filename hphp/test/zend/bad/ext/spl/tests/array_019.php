<?php

$ar = new ArrayObject(array(1));            foreach($ar as &$v) var_dump($v);
$ar = new ArrayIterator(array(2));          foreach($ar as &$v) var_dump($v);
$ar = new RecursiveArrayIterator(array(3)); foreach($ar as &$v) var_dump($v);

class ArrayIteratorEx extends ArrayIterator
{
	function current()
	{
		return ArrayIterator::current();
	}
}

$ar = new ArrayIteratorEx(array(4)); foreach($ar as $v) var_dump($v);
$ar = new ArrayIteratorEx(array(5)); foreach($ar as &$v) var_dump($v);

?>
===DONE===
<?php exit(0); ?>