<?php

class myPriorityQueue extends SplPriorityQueue{

	public function compare($a, $b){
	 	 if ($b == 2) {
		throw new Exception('ignore me');
		}  else {
		return parent::compare($a, $b);
		}
	}
}

$priorityQueue = new myPriorityQueue();
$priorityQueue->insert("a", 1);

try {
    //corrupt heap
    $priorityQueue->insert("b", 2);
        // ignore exception tested elsewhere
} catch (Exception $e) {
}

try {
    $priorityQueue->top();
} catch (RuntimeException $e) {
  echo "Exception: ".$e->getMessage().PHP_EOL;
}

?>