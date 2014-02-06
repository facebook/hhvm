<?php

class myHeap extends SplHeap
{
	protected function compare($v1, $v2)
	{
		throw new Exception('');
	}
}

$heap = new myHeap();
var_dump($heap->current());

?>
