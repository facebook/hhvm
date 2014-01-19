<?php

class myHeap extends SplHeap
{
	public $allow_compare = true;
	
	public function compare($v1, $v2)
	{
		if ($this->allow_compare == true)
		{
			if ($v1 > $v2)
			{
				return 1;
			}
			else if ($v1 < $v2)
			{
				return -1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			throw new Exception('Compare exception');
		}
	}
}

$heap = new myHeap();
$heap->insert(1);
$heap->insert(2);
$heap->insert(3);
$heap->insert(4);

$heap->allow_compare = false;

try {
	$heap->extract();
}
catch (Exception $e) {
	echo "Compare Exception: " . $e->getMessage() . PHP_EOL;
}

try {
	$heap->top();
}
catch (Exception $e) {
	echo "Corruption Exception: " . $e->getMessage() . PHP_EOL;
}

?>